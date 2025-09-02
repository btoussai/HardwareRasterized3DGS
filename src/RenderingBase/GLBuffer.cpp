//
// Created by Briac on 18/06/2025.
//

#include "GLBuffer.h"
#include "GLIntrospection.h"
#include "helper_cuda.h"
#include <cassert>

GLBuffer::~GLBuffer() {
    reset();
}

void GLBuffer::reset() {
    if(!initialized) return;

    GLIntrospection::removeBuffer(ID);
    if(useCudaGL_interop && cudaResource){
        // this can throw if the buffer is mapped
        CudaGLInterop::unregisterBuffer(&cudaResource);
    }
    glDeleteBuffers(1, &ID);
    ID = 0;
    numElements = 0;
    elementSize = 1;
    sizeInBytes = 0;

    flags = 0;
    gl_ptr = 0;

    useCudaGL_interop = false;
    cudaResource = nullptr;
    cuda_ptr = nullptr;

    initialized = false;
}

GLBuffer::GLBuffer(GLBuffer && buff) noexcept : GLBuffer() {
    *this = std::move(buff);
}

GLBuffer &GLBuffer::operator=(GLBuffer && buff) {
    if(this == &buff){
        return *this;
    }

    reset();

    this->ID = buff.ID;
    this->numElements = buff.numElements;
    this->sizeInBytes = buff.sizeInBytes;
    this->elementSize = buff.elementSize;
    this->flags = buff.flags;
    this->gl_ptr = buff.gl_ptr;
    this->useCudaGL_interop = buff.useCudaGL_interop;
    this->cudaResource = buff.cudaResource;
    this->cuda_ptr = buff.cuda_ptr;
    this->initialized = buff.initialized;

    buff.ID = 0;
    buff.elementSize = 1;
    buff.sizeInBytes = 0;
    buff.numElements = 0;
    buff.flags = 0;
    buff.gl_ptr = 0;
    buff.useCudaGL_interop = false;
    buff.cudaResource = nullptr;
    buff.cuda_ptr = nullptr;
    buff.initialized = false;

    return *this;
}

void GLBuffer::storeData(const void *data, size_t numElements, size_t elementSize, int flags, bool useCudaGLInterop,
                         bool init_zero, bool makeResident) {
    reset();
    initialized = true;

    glCreateBuffers(1, &ID);
    GLIntrospection::addBuffer(ID);

    glNamedBufferStorage(ID, numElements * elementSize, data, flags);

    this->flags = flags;
    this->numElements = numElements;
    this->elementSize = elementSize;
    this->sizeInBytes = numElements * elementSize;

    this->useCudaGL_interop = useCudaGLInterop;
    if(useCudaGLInterop){
        cudaResource = CudaGLInterop::registerBuffer(ID);

        size_t num_bytes = 0;
        checkCudaErrors(cudaGraphicsMapResources(1, &cudaResource));
        checkCudaErrors(cudaGraphicsResourceGetMappedPointer(&cuda_ptr, &num_bytes, cudaResource));
        checkCudaErrors(cudaGraphicsUnmapResources(1, &cudaResource));
    }

    // make the buffer bindless by default
    //can take a long time for large buffers
    glGetNamedBufferParameterui64vNV(ID, GL_BUFFER_GPU_ADDRESS_NV, &gl_ptr);
    if(makeResident){
        // this is optional as we may create the buffer on a shared context, on which it may not need to be resident
        makeBufferResident(GL_READ_WRITE);
    }

    if(init_zero){
        int zero = 0;
        glClearNamedBufferData(ID, GL_R32I, GL_RED_INTEGER, GL_INT, &zero);
    }

}

void GLBuffer::bindAs(GLuint type){
    glBindBuffer(type, ID);
}
void GLBuffer::unbindAs(GLuint type){
    glBindBuffer(type, 0);
}

void GLBuffer::makeBufferResident(GLenum access, bool checkNonResident) {
    // Optionally check if the buffer is already resident.
    if(!checkNonResident || !glIsNamedBufferResidentNV(ID)) {
        // If resident, this will create an error.
        glMakeNamedBufferResidentNV(ID, access);
    }
}

void GLBuffer::makeBufferNonResident() {
    glMakeNamedBufferNonResidentNV(ID);
}

void GLBuffer::clearData(GLenum internalformat, GLenum format, GLenum type, const void *data) {
    glClearNamedBufferSubData(ID, internalformat, 0, numElements * elementSize, format, type, data);
}

void GLBuffer::updateData(const void *data, size_t numElements, size_t elementSize, size_t elementOffset) {
    assert(this->elementSize == elementSize);
    assert(this->numElements >= numElements);
    glNamedBufferSubData(ID, elementOffset * elementSize, numElements * elementSize, data);
}

void GLBuffer::printHead(std::string title, int n) {

    std::vector<float> vec(n, 0.0f);
    checkCudaErrors(cudaMemcpy(vec.data(), cuda_ptr, n * sizeof(float), cudaMemcpyDeviceToHost));
    std::cout <<title <<", first " <<n <<" elements:" <<std::endl;
    for(int i=0; i<n; i++){
        std::cout <<vec[i] <<" ";
    }
    std::cout <<std::endl;


}

std::vector<float> GLBuffer::getAsFloats(int n) {
    std::vector<float> vec(n, 0.0f);
    checkCudaErrors(cudaMemcpy(vec.data(), cuda_ptr, n * sizeof(float), cudaMemcpyDeviceToHost));
    return vec;
}

