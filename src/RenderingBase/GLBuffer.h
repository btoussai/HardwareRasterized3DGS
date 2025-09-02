//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_GLBUFFER_H
#define SPARSEVOXRECON_GLBUFFER_H

#include "../glad/gl.h"
#include "CudaGLInterop.h"
#include <vector>

class GLBuffer {
public:
    GLBuffer() = default;
    virtual ~GLBuffer();

    GLBuffer(GLBuffer&&) noexcept;
    GLBuffer& operator=(GLBuffer&&);

    GLBuffer(const GLBuffer&) = delete;
    GLBuffer& operator=(const GLBuffer&) = delete;

    void reset();

    void storeData(const void* data, size_t numElements, size_t elementSize, int flags=0, bool useCudaGLInterop=false, bool init_zero=false, bool makeResident=true);
    void updateData(const void* data, size_t numElements, size_t elementSize, size_t elementOffset);
    void clearData(GLenum internalformat, GLenum format, GLenum type, const void *data);

    GLuint getID() const{
        return ID;
    }
    int64_t getNumElements() const{
        return numElements;
    }
    uint64_t getSizeInBytes() const{
        return sizeInBytes;
    }
    uint64_t getElementSize() const{
        return elementSize;
    }
    int getFlags() const{
        return flags;
    }
    uint64_t getGLptr() const{
        return gl_ptr;
    }
    cudaGraphicsResource_t& getCudaResource(){
        return cudaResource;
    }
    void* getCudaPtr() const{
        return cuda_ptr;
    }

    void bindAs(GLuint type);
    void unbindAs(GLuint type);

    void makeBufferResident(GLenum access, bool checkNonResident=false);
    void makeBufferNonResident();

    void printHead(std::string title, int n);
    std::vector<float> getAsFloats(int n);

private:
    GLuint ID = 0;
    int64_t numElements = 0;
    uint64_t sizeInBytes = 0;
    uint64_t elementSize = 1;

    int flags = 0;
    uint64_t gl_ptr = 0;

    bool useCudaGL_interop = false;
    cudaGraphicsResource_t cudaResource = nullptr;
    void* cuda_ptr = nullptr;

    bool initialized = false;
};

#endif //SPARSEVOXRECON_GLBUFFER_H
