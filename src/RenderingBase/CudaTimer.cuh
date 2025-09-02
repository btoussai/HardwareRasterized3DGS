//
// Created by Briac on 03/07/2025.
//

#ifndef SPARSEVOXRECON_CUDATIMER_CUH
#define SPARSEVOXRECON_CUDATIMER_CUH


class CudaTimer {
public:
    CudaTimer();
    virtual ~CudaTimer();
    void start();
    void stop();
    float getTimeMs();
private:
    cudaEvent_t start_time, stop_time;
    int calls=0;
    double total_ms=0.0f;
    bool new_measure=false;
};


#endif //SPARSEVOXRECON_CUDATIMER_CUH
