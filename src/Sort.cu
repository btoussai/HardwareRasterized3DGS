//
// Created by Briac on 28/08/2025.
//

#include "Sort.cuh"

#include "RenderingBase/CudaBuffer.cuh"
#include <cub/cub.cuh>

static CudaBuffer<char> temp;

void Sort::sort(GLBuffer &depths, GLBuffer &sorted_depths, GLBuffer &indices, GLBuffer &sorted_indices, int count) {

    checkCudaErrors(cudaGraphicsMapResources(1, &depths.getCudaResource()));

    CudaBuffer<int> keys_in = CudaBuffer<int>::fromGLBuffer(depths);
    CudaBuffer<int> keys_out = CudaBuffer<int>::fromGLBuffer(sorted_depths);

    CudaBuffer<int> values_in = CudaBuffer<int>::fromGLBuffer(indices);
    CudaBuffer<int> values_out = CudaBuffer<int>::fromGLBuffer(sorted_indices);

    size_t temp_storage_bytes;
    cub::DeviceRadixSort::SortPairs(
            nullptr, temp_storage_bytes,
            keys_in.ptr, keys_out.ptr, // keys
            values_in.ptr, values_out.ptr, // values
            count);

    if(temp.numElements < temp_storage_bytes){
        temp = CudaBuffer<char>::allocate(int(temp_storage_bytes), "RadixSort::TempStorage");
    }

    cub::DeviceRadixSort::SortPairs(
            temp.ptr, temp_storage_bytes,
            keys_in.ptr, keys_out.ptr, // keys
            values_in.ptr, values_out.ptr, // values
            count);

    checkCudaErrors(cudaGraphicsUnmapResources(1, &depths.getCudaResource()));
}
