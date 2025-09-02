//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_VAO_H
#define SPARSEVOXRECON_VAO_H

#include <vector>
#include "GLBuffer.h"
#include <memory>
#include <unordered_map>

class VAO {
public:
    VAO();
    virtual ~VAO();

    VAO(VAO&& vao) noexcept;
    VAO& operator=(VAO&&);

    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    void bind() const;
    void unbind() const;

    void bindAttributes(const std::initializer_list<uint32_t>& attributes) const;
    void unbindAttributes(const std::initializer_list<uint32_t>& attributes) const;

    uint32_t getIndexCount() const;
    GLBuffer* getIndices();
    GLBuffer* getVBO(int attribute);

    void createIndexBuffer(const std::vector<int>& indices, bool cudaGLInterop=false);
    void createContiguousFloatAttribute(uint32_t attribNumber,
                                        const std::vector<float>& data, uint32_t attribSize, std::size_t offset=0, bool cudaGLInterop=false);

private:
    GLuint ID = 0;
    std::unique_ptr<GLBuffer> indexVBO; // Optional
    uint32_t indexCount;
    std::unordered_map<uint32_t , std::unique_ptr<GLBuffer>> vbos;
};

#endif //SPARSEVOXRECON_VAO_H
