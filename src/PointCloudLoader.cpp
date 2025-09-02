//
// Created by Briac on 27/08/2025.
//

#include "PointCloudLoader.h"

#include <vector>

#include "glm/vec3.hpp"
#include "glm/common.hpp"
#include "RenderingBase/VAO.h"

#include "miniply/miniply.h"

using namespace glm;

static const char *kFileTypes[] = {
        "ascii",
        "binary_little_endian",
        "binary_big_endian",
};
static const char *kPropertyTypes[] = {
        "char",
        "uchar",
        "short",
        "ushort",
        "int",
        "uint",
        "float",
        "double",
};


static bool has_extension(const char *filename, const char *ext) {
    int j = int(strlen(ext));
    int i = int(strlen(filename)) - j;
    if (i <= 0 || filename[i - 1] != '.') {
        return false;
    }
    return strcmp(filename + i, ext) == 0;
}


bool print_ply_header(const char *filename) {
    miniply::PLYReader reader(filename);
    if (!reader.valid()) {
        fprintf(stderr, "Failed to open %s\n", filename);
        return false;
    }

    printf("ply\n");
    printf("format %s %d.%d\n", kFileTypes[int(reader.file_type())], reader.version_major(), reader.version_minor());
    for (uint32_t i = 0, endI = reader.num_elements(); i < endI; i++) {
        const miniply::PLYElement *elem = reader.get_element(i);
        printf("element %s %u\n", elem->name.c_str(), elem->count);
        for (const miniply::PLYProperty &prop: elem->properties) {
            if (prop.countType != miniply::PLYPropertyType::None) {
                printf("property list %s %s %s\n", kPropertyTypes[uint32_t(prop.countType)],
                       kPropertyTypes[uint32_t(prop.type)], prop.name.c_str());
            } else {
                printf("property %s %s\n", kPropertyTypes[uint32_t(prop.type)], prop.name.c_str());
            }
        }
    }
    printf("end_header\n");

    while (reader.has_element()) {
        const miniply::PLYElement *elem = reader.element();
        if (elem->fixedSize || elem->count == 0) {
            reader.next_element();
            continue;
        }

        if (!reader.load_element()) {
            fprintf(stderr, "Element %s failed to load\n", elem->name.c_str());
        }
        for (const miniply::PLYProperty &prop: elem->properties) {
            if (prop.countType == miniply::PLYPropertyType::None) {
                continue;
            }
            bool mixedSize = false;
            const uint32_t firstRowCount = prop.rowCount.front();
            for (const uint32_t rowCount: prop.rowCount) {
                if (rowCount != firstRowCount) {
                    mixedSize = true;
                    break;
                }
            }
            if (mixedSize) {
                printf("Element '%s', list property '%s': not all lists have the same size\n",
                       elem->name.c_str(), prop.name.c_str());
            } else {
                printf("Element '%s', list property '%s': all lists have size %u\n",
                       elem->name.c_str(), prop.name.c_str(), firstRowCount);
            }
        }
        reader.next_element();
    }

    return true;
}


//static std::vector<float> buildPositions(aiMesh *mesh,
//                                         bool scaleToUnit) {
//    uint32_t numVertices = mesh->mNumVertices;
//
//    std::vector<float> vertices(numVertices * 3, 0.0f);
//    vec3 Vmin = vec3(+INFINITY);
//    vec3 Vmax = vec3(-INFINITY);
//
//    for (uint32_t v = 0; v < numVertices; v++) {
//        auto vertex = mesh->mVertices[v];
//        vertices[3 * v + 0] = vertex.x;
//        vertices[3 * v + 1] = vertex.y;
//        vertices[3 * v + 2] = vertex.z;
//
//        vec3 u(vertex.x, vertex.y, vertex.z);
//        Vmax = max(Vmax, u);
//        Vmin = min(Vmin, u);
//    }
//
//    if (scaleToUnit) {
//        vec3 size = Vmax - Vmin;
//        vec3 center = Vmin + size * 0.5f;
//        float half_extent = std::max(std::max(size.x, size.y), size.z) * 0.5f;
//        for (uint32_t v = 0; v < numVertices; v++) {
//            vertices[3 * v + 0] = (vertices[3 * v + 0] - center.x) / half_extent;
//            vertices[3 * v + 1] = (vertices[3 * v + 1] - center.y) / half_extent;
//            vertices[3 * v + 2] = (vertices[3 * v + 2] - center.z) / half_extent;
//        }
//    }
//
//    return vertices;
//}

float sigmoid(float x){
    return 1.0f / (1.0f + exp(-x));
}

void PointCloudLoader::load(GaussianCloud& dst, const std::string &path, bool useCudaGLInterop) {
    dst.initialized = false;

    std::cout << "Loading point cloud: " << path << " ..." << std::endl;

    print_ply_header(path.c_str());
    std::cout << "End of header."<< std::endl;

    miniply::PLYReader reader(path.c_str());
    if (!reader.valid()) {
        std::cout << "Couldn't read " <<path << std::endl;
        return;
    }

    assert(reader.has_element());

    const miniply::PLYElement *elem = reader.element();

    if (!reader.load_element()) {
        std::cout <<"Element" <<elem->name <<" failed to load." <<std::endl;
        return;
    }

    assert(elem->name == "vertex");

    dst.num_gaussians = (int)elem->count;

    const uint pos_idx[3] = {
            elem->find_property("x"),
            elem->find_property("y"),
            elem->find_property("z")
    };
    const uint rot_idx[4] = {
            elem->find_property("rot_0"),
            elem->find_property("rot_1"),
            elem->find_property("rot_2"),
            elem->find_property("rot_3")
    };
    const uint scale_idx[3] = {
            elem->find_property("scale_0"),
            elem->find_property("scale_1"),
            elem->find_property("scale_2")
    };
    const uint opacity_idx[1] = {
            elem->find_property("opacity")
    };

    dst.positions_cpu = std::vector<glm::vec4>(dst.num_gaussians);
    for(int i=0; i<dst.num_gaussians; i++){
        dst.positions_cpu[i].w = 1.0f;
    }
    reader.extract_properties_with_stride(pos_idx, 3, miniply::PLYPropertyType::Float, dst.positions_cpu.data(), 4*sizeof(float));
    dst.positions.storeData(dst.positions_cpu.data(), dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, false, true);

    dst.scales_cpu = std::vector<glm::vec4>(dst.num_gaussians);
    reader.extract_properties_with_stride(scale_idx, 3, miniply::PLYPropertyType::Float, dst.scales_cpu.data(), 4*sizeof(float));
    for(int i=0; i<dst.num_gaussians; i++){
        dst.scales_cpu[i] = exp(dst.scales_cpu[i]); // apply exponential activation
        dst.scales_cpu[i].w = 0.0f;
    }
    dst.scales.storeData(dst.scales_cpu.data(), dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, false, true);

    dst.rotations_cpu = std::vector<glm::vec4>(dst.num_gaussians);
    reader.extract_properties(rot_idx, 4, miniply::PLYPropertyType::Float, dst.rotations_cpu.data());
    dst.rotations.storeData(dst.rotations_cpu.data(), dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, false, true);

    dst.opacities_cpu = std::vector<float>(dst.num_gaussians);
    reader.extract_properties(opacity_idx, 1, miniply::PLYPropertyType::Float, dst.opacities_cpu.data());
    for(int i=0; i<dst.num_gaussians; i++){
        dst.opacities_cpu[i] = sigmoid(dst.opacities_cpu[i]); // apply sigmoid activation
    }
    dst.opacities.storeData(dst.opacities_cpu.data(), dst.num_gaussians, 1*sizeof(float), 0, useCudaGLInterop, false, true);

    uint sh_idx[48];
    for(int i=0; i<48; i++){
        const std::string prop_name = i < 3 ? "f_dc_" + std::to_string(i) : "f_rest_" + std::to_string(i-3);
        sh_idx[i] = elem->find_property(prop_name.c_str());
    }

    for(int i=0; i<3; i++) {
        float* sh_coeffs = new float[dst.num_gaussians * 16];
        uint channel_idx[16];
        for(int j=0; j<16; j++){
//            channel_idx[j] = sh_idx[j*3+i];
            if(j == 0){
                channel_idx[0] = sh_idx[i];
            }else{
                channel_idx[j] = sh_idx[3+i*15+j-1];
            }
        }
        reader.extract_properties(channel_idx, 16, miniply::PLYPropertyType::Float, sh_coeffs);
        dst.sh_coeffs[i].storeData(sh_coeffs, dst.num_gaussians, 16*sizeof(float), 0, useCudaGLInterop, false, true);
        delete[] sh_coeffs;
    }

    dst.visible_gaussians_counter.storeData(nullptr, 1, sizeof(int), 0, useCudaGLInterop, false, true);
    dst.gaussians_depths.storeData(nullptr, dst.num_gaussians, sizeof(float), 0, useCudaGLInterop, true, true);
    dst.gaussians_indices.storeData(nullptr, dst.num_gaussians, sizeof(int), 0, useCudaGLInterop, true, true);
    dst.sorted_depths.storeData(nullptr, dst.num_gaussians, sizeof(float), 0, useCudaGLInterop, true, true);
    dst.sorted_gaussian_indices.storeData(nullptr, dst.num_gaussians, sizeof(int), 0, useCudaGLInterop, true, true);

    dst.bounding_boxes.storeData(nullptr, dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, true, true);
    dst.conic_opacity.storeData(nullptr, dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, true, true);
    dst.eigen_vecs.storeData(nullptr, dst.num_gaussians, 2*sizeof(float), 0, useCudaGLInterop, true, true);
    dst.predicted_colors.storeData(nullptr, dst.num_gaussians, 4*sizeof(float), 0, useCudaGLInterop, true, true);

    dst.initialized = true;

    std::cout << "Finished loading point cloud." << std::endl;
}
