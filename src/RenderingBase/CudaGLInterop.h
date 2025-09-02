#ifndef SRC_RENDERINGBASE_CUDAGLINTEROP_H
#define SRC_RENDERINGBASE_CUDAGLINTEROP_H

#include <cuda_runtime_api.h>
#include <iostream>

class CudaGLInterop {
public:
    static cudaGraphicsResource_t registerBuffer(unsigned int id);
    static cudaGraphicsResource_t registerImage(unsigned int id, unsigned int target);
    static void unregisterBuffer(cudaGraphicsResource_t* resource);
    static void unregisterImage(cudaGraphicsResource_t* resource);
};

#endif //SRC_RENDERINGBASE_CUDAGLINTEROP_H
