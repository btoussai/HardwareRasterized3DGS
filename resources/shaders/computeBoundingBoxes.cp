//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_KHR_shader_subgroup_ballot : enable


/*-- layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in; --*/

#include "./common/GLSLDefines.h"
#include "./common/Uniforms.h"
#include "./common/Covariance.h"

void main(void){
    const int n = int(gl_GlobalInvocationID.x);
    if(n >= *uniforms.visible_gaussians_counter)
        return;

    const int GaussianID = uniforms.sorted_gaussian_indices[n];

    const float opacity = uniforms.opacities[GaussianID];
    const vec3 mean_world_space = vec3(uniforms.positions[GaussianID]);
    const vec3 scale = vec3(uniforms.scales[GaussianID]);
    const vec4 quaternion = uniforms.rotations[GaussianID];

    const float scale_modifier = uniforms.scale_modifier;

    const float width = uniforms.width;
    const float height = uniforms.height;
    const float focal_x = uniforms.focal_x;
    const float focal_y = uniforms.focal_y;

    // transform to view space
    const vec3 mean = vec3(uniforms.viewMat * vec4(mean_world_space, 1.0f));
    const mat3 cov3D = computeCov3D(scale, scale_modifier, quaternion, mat3(uniforms.viewMat));

    const vec4 p_hom = uniforms.projMat * vec4(mean, 1.0f);
    const vec2 ndc = vec2(p_hom) / p_hom.w;

    // Compute 2D screen-space covariance matrix
    vec3 cov = computeCov2D(mean, focal_x, focal_y, cov3D);

    const float h_var = 0.3f;
    const float det_cov = cov.x * cov.z - cov.y * cov.y;
    cov.x += h_var;
    cov.z += h_var;
    const float det_cov_plus_h_cov = cov.x * cov.z - cov.y * cov.y;
    float h_convolution_scaling = 1.0f;

    if(uniforms.antialiasing > 0)
        h_convolution_scaling = sqrt(max(0.000025f, det_cov / det_cov_plus_h_cov)); // max for numerical stability

    const float det = det_cov_plus_h_cov;

    if (det == 0.0f)
        return;

    const float det_inv = 1.0f / det;
    const vec3 conic = vec3( cov.z * det_inv, -cov.y * det_inv, cov.x * det_inv );

    vec2 eigen_vec = vec2(1.0f, 0.0f);
    const vec2 bbox_pixels = computeOBB(conic, opacity * h_convolution_scaling, uniforms.min_opacity, eigen_vec);

    const vec2 proj_pixels = vec2(ndc * 0.5f + 0.5f) * vec2(width, height);
    const vec4 bounding_box = vec4(proj_pixels, bbox_pixels);
    const vec4 conic_opacity = vec4( conic.x, conic.y, conic.z, opacity * h_convolution_scaling);

    uniforms.bounding_boxes[n] = bounding_box;
    uniforms.conic_opacity[n] = conic_opacity;
    uniforms.eigen_vecs[n] = eigen_vec;

        // project point to image plane
//        const float u = mean.x / -mean.z;
//        const float v = mean.y / -mean.z;
//        const float z = mean.z;
//
//        const vec3 ray = vec3(u, v, 1.0f);

//        mat2 cov2D = computeCov2D_Me(mean, cov3D, ray);
//        const vec2 half_extent = computeAABB(cov2D, opacity, uniforms.min_opacity);

        // convert to ndc
//        const vec2 ratios = vec2(uniforms.projMat[0][0], uniforms.projMat[1][1]);
//        const vec4 bbox = vec4(vec2(u, v) * ratios, half_extent * ratios);

//        const mat2 S = mat2(1.0f / ratios.x, 0.0f, 0.0f, 1.0f / ratios.y);
//        cov2D = S * cov2D * S;

//        uniforms.bounding_boxes[n] = bbox;
//        uniforms.covariance_matrices_2d[n] = vec4(cov2D[0][0], cov2D[0][1], cov2D[1][1], opacity);


}