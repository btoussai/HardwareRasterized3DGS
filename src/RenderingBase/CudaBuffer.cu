//
// Created by Briac on 28/07/2025.
//

#include "CudaBuffer.cuh"
#include <iostream>
#include <unordered_map>

__constant__ BoundsErrorArray boundsErrorArray;

static BoundsErrorArray array = {};

void CudaBufferSetupBoundsCheck() {

    if (array.errors == nullptr) {
        int maxErrors = 10000;
        cudaMalloc((BoundsError **) &array.errors, sizeof(BoundsError) * maxErrors);
        cudaMalloc((int **) &array.count, sizeof(int));
        array.errors_size = maxErrors;
        cudaMemcpyToSymbol(boundsErrorArray, &array, sizeof(BoundsError), 0);
    }

}

std::string strDeviceToHost(const char* ptr){
    std::string s;

    int i = 0;
    while(true){
        char c;
        cudaMemcpy(&c, ptr+i, 1, cudaMemcpyDeviceToHost);
        if(c == '\0')
            break;
        s += c;
        i++;
    }

    return s;
}

void CudaBufferProcessBoundsCheckErrors() {

    if(!REPORT_OOB){
        return;
    }

    checkCudaErrors(cudaDeviceSynchronize());

    std::unordered_map<const char*, std::string> m;

    int num_errors = 0;
    cudaMemcpy(&num_errors, array.count, sizeof(int), cudaMemcpyDeviceToHost);
    if (num_errors != 0) {
        std::vector<BoundsError> errors(std::min(num_errors, array.errors_size));
        cudaMemcpy(errors.data(), array.errors, errors.size() * sizeof(BoundsError), cudaMemcpyDeviceToHost);

        std::cout << "CudaBuffer: " << num_errors << " out of bounds read / write detected!" << std::endl;
        for (int i = 0; i < std::min(10ull, errors.size()); i++) {
            if(!m.contains(errors[i].file)){
                m[errors[i].file] = strDeviceToHost(errors[i].file);
            }
            std::cout <<"Out of bounds detected in CudaBuffer: " <<m[errors[i].file] <<"(" <<errors[i].line <<")" <<" index: " <<errors[i].index <<" / " <<errors[i].elements <<" elements." <<std::endl;
        }

        exit(-1);
    }

}

