#include "CudaGLInterop.h"

#include "../glad/gl.h"
#include <cuda_gl_interop.h>
#include "helper_cuda.h"

cudaGraphicsResource_t CudaGLInterop::registerBuffer(unsigned int id){
    cudaGraphicsResource_t res = nullptr;
    checkCudaErrors(cudaGraphicsGLRegisterBuffer(&res, id, cudaGraphicsRegisterFlagsNone));
    return res;
}
void CudaGLInterop::unregisterBuffer(cudaGraphicsResource_t* resource){
    checkCudaErrors(cudaGraphicsUnregisterResource(*resource));
    resource = nullptr;
}

cudaGraphicsResource_t CudaGLInterop::registerImage(unsigned int id, unsigned int target) {
    cudaGraphicsResource_t res = nullptr;
    checkCudaErrors(cudaGraphicsGLRegisterImage(&res, id, target, cudaGraphicsRegisterFlagsSurfaceLoadStore));
    return res;
}

void CudaGLInterop::unregisterImage(cudaGraphicsResource_t* resource){
    checkCudaErrors(cudaGraphicsUnregisterResource(*resource));
    resource = nullptr;
}
