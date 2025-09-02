//
// Created by Briac on 19/06/2025.
//

#include "MeshLoader.h"

#include <cassert>
#include <vector>
#include <bit>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include "../glm/vec3.hpp"
#include "../glm/common.hpp"

using namespace glm;

static std::vector<int> buildIndices(aiMesh *mesh) {
    const int numIndices = mesh->mNumFaces * 3;

    std::vector<int> indices(numIndices, 0);
    for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
        auto face = mesh->mFaces[f];
        indices[3 * f + 0] = face.mIndices[0];
        indices[3 * f + 1] = face.mIndices[1];
        indices[3 * f + 2] = face.mIndices[2];
    }

    return indices;
}

static std::vector<float> buildPositions(aiMesh *mesh,
                             bool scaleToUnit) {
    uint32_t numVertices = mesh->mNumVertices;

    std::vector<float> vertices(numVertices * 3, 0.0f);
    vec3 Vmin = vec3(+INFINITY);
    vec3 Vmax = vec3(-INFINITY);

    for (uint32_t v = 0; v < numVertices; v++) {
        auto vertex = mesh->mVertices[v];
        vertices[3 * v + 0] = vertex.x;
        vertices[3 * v + 1] = vertex.y;
        vertices[3 * v + 2] = vertex.z;

        vec3 u(vertex.x, vertex.y, vertex.z);
        Vmax = max(Vmax, u);
        Vmin = min(Vmin, u);
    }

    if (scaleToUnit) {
        vec3 size = Vmax - Vmin;
        vec3 center = Vmin + size * 0.5f;
        float half_extent = std::max(std::max(size.x, size.y), size.z) * 0.5f;
        for (uint32_t v = 0; v < numVertices; v++) {
            vertices[3 * v + 0] = (vertices[3 * v + 0] - center.x) / half_extent;
            vertices[3 * v + 1] = (vertices[3 * v + 1] - center.y) / half_extent;
            vertices[3 * v + 2] = (vertices[3 * v + 2] - center.z) / half_extent;
        }
    }

    return vertices;
}


static std::vector<float> buildNormals(aiMesh *mesh) {
    uint32_t numNormals = mesh->mNumVertices;
    std::vector<float> normals(numNormals * 3, 0.0f);
    for (uint32_t v = 0; v < numNormals; v++) {
        auto normal = mesh->mNormals[v];
        normals[3 * v + 0] = normal.x;
        normals[3 * v + 1] = normal.y;
        normals[3 * v + 2] = normal.z;
    }
    return normals;
}

static std::vector<float> buildTexCoords(aiMesh *mesh) {
    uint32_t numTexture = mesh->mNumVertices;
    std::vector<float> uvs(numTexture * 2, 0.0f);

    for (uint32_t i = 0; i < numTexture; i++) {
        auto tex = mesh->mTextureCoords[0][i];
        uvs[2 * i + 0] = tex.x;
        uvs[2 * i + 1] = tex.y;
    }

    return uvs;
}

VAO MeshLoader::loadMesh(const std::string &path, bool cudaGLInterop) {
    VAO vao;

    const int flags = 0;
    std::string fullpath = "resources/meshes/" + path;
    const bool scale_to_unit = true;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fullpath,
                              aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | flags);

    if (!scene || scene->mNumMeshes == 0) {
        std::cout <<"Failed to load mesh: " <<fullpath <<std::endl;
        return vao;
    }
    aiMesh *mesh = scene->mMeshes[0];
    assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

    if (mesh->HasFaces()) {
        auto indices = buildIndices(mesh);
        vao.createIndexBuffer(indices, cudaGLInterop);
    }

    if (mesh->HasPositions()) {
        auto vertices = buildPositions(mesh, scale_to_unit);
        vao.createContiguousFloatAttribute(0, vertices, 3, 0, cudaGLInterop);
        std::cout <<"Positions detected in mesh" <<std::endl;
    }

    if (mesh->HasNormals()) {
        auto normals = buildNormals(mesh);
        vao.createContiguousFloatAttribute(1, normals, 3, 0, cudaGLInterop);
        std::cout <<"Normals detected in mesh" <<std::endl;
    }

    if (mesh->HasTextureCoords(0)) {
        auto uvs = buildTexCoords(mesh);
        vao.createContiguousFloatAttribute(2, uvs, 2, 0, cudaGLInterop);
        std::cout <<"UVs detected in mesh" <<std::endl;
    }

    return vao;
}
