//
// Created by Briac on 03/07/2025.
//

#include "CudaTimer.cuh"

#include "helper_cuda.h"
#include <cuda_runtime.h>

CudaTimer::CudaTimer() {
    checkCudaErrors(cudaEventCreate(&start_time));
    checkCudaErrors(cudaEventCreate(&stop_time));
}

CudaTimer::~CudaTimer() {
    checkCudaErrors(cudaEventDestroy(start_time));
    checkCudaErrors(cudaEventDestroy(stop_time));
}

void CudaTimer::start() {
    checkCudaErrors(cudaEventRecord(start_time));
}

void CudaTimer::stop() {
    checkCudaErrors(cudaEventRecord(stop_time));
    new_measure = true;
}

float CudaTimer::getTimeMs() {
    float elapsedTime_ms = 0.0f;
    auto error = cudaEventElapsedTime(&elapsedTime_ms, start_time, stop_time);
    if(error == cudaErrorInvalidResourceHandle){
        // timer not started yet.
    }else if(error == cudaErrorNotReady){
        // result not yet available
    }else{
        checkCudaErrors(error);
    }

    // ema
    const double beta = 0.99;
    if(elapsedTime_ms > 0.0f && new_measure){
        calls++;
        total_ms = total_ms * beta + elapsedTime_ms * (1.0 - beta);
        new_measure = false;
    }
    const double normalization = 1.0 / (1.0 - pow(beta, calls));
    return calls > 0 ? float(total_ms * normalization) : 0.0f;
}
