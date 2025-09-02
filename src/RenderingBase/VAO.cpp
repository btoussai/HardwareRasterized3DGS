//
// Created by Briac on 18/06/2025.
//

#include "VAO.h"
#include "GLIntrospection.h"
#include "GLBuffer.h"
#include <cassert>

VAO::VAO() {
    glCreateVertexArrays (1, &ID);
    GLIntrospection::addVAO(ID);
    indexCount = 0;
}

VAO::~VAO() {
    GLIntrospection::removeVAO(ID);
    glDeleteVertexArrays(1, &ID);
}

VAO::VAO(VAO &&vao) noexcept {
    *this = std::move(vao);
}

VAO &VAO::operator=(VAO && vao) {
    if(this == &vao){
        return *this;
    }

    this->indexCount = vao.indexCount;
    this->indexVBO = std::move(vao.indexVBO);
    vbos = std::move(vao.vbos);

    GLIntrospection::removeVAO(ID);
    glDeleteVertexArrays(1, &ID);
    this->ID = vao.ID;

    vao.indexCount = 0;
    vao.ID = 0;

    return *this;
}

void VAO::bind() const {
    glBindVertexArray(ID);
}

void VAO::unbind() const {
    glBindVertexArray(0);
}

void VAO::bindAttributes(const std::initializer_list<uint32_t> &attributes) const {
    for(uint32_t attribute : attributes){
        glEnableVertexArrayAttrib(ID, attribute);
    }
}

void VAO::unbindAttributes(const std::initializer_list<uint32_t>& attributes) const {
    for(uint32_t attribute : attributes){
        glDisableVertexArrayAttrib(ID, attribute);
    }
}

uint32_t VAO::getIndexCount() const {
    return indexCount;
}

void VAO::createIndexBuffer(const std::vector<int> &indices, bool cudaGLInterop) {
    assert(indexVBO == nullptr);
    indexVBO = std::make_unique<GLBuffer>();
    indexCount = indices.size();
    indexVBO->storeData(indices.data(), indexCount, sizeof(uint32_t), 0, cudaGLInterop);
    glVertexArrayElementBuffer(ID, indexVBO->getID());
}

void VAO::createContiguousFloatAttribute(uint32_t attribNumber,
                                         const std::vector<float>& data, uint32_t attribSize,
                                         std::size_t offset, bool cudaGLInterop)
{
    if(vbos.contains(attribNumber)){
        throw std::string("Error, vertex attribute ") + std::to_string(attribNumber) + std::string(" already exists.");
    }

    std::unique_ptr<GLBuffer> vbo = std::make_unique<GLBuffer>();
    vbo->storeData(data.data(), data.size(), sizeof(float), 0, cudaGLInterop);

    glVertexArrayVertexBuffer(ID, attribNumber, vbo->getID(), 0, attribSize * sizeof(float));
    glVertexArrayAttribFormat(ID, attribNumber, attribSize, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(ID, attribNumber, attribNumber);

    glEnableVertexArrayAttrib(ID, attribNumber);

    vbos.insert(std::pair<uint32_t, std::unique_ptr<GLBuffer>>(attribNumber, std::move(vbo)));
}

GLBuffer* VAO::getIndices() {
    return indexVBO.get();
}

GLBuffer* VAO::getVBO(int attribute) {
    auto p = vbos.find(attribute);
    if(p == vbos.end()){
        return nullptr;
    }else{
        return p->second.get();
    }
}
