//
// Created by Briac on 20/06/2025.
//

#ifndef SPARSEVOXRECON_CUDABUFFER_CUH
#define SPARSEVOXRECON_CUDABUFFER_CUH

#include "GLBuffer.h"
#include "helper_cuda.h"
#include "CudaCommon.cuh"
#include "CudaIntrospection.cuh"
#include <cuda.h>
#include <iostream>
#include <vector>

struct cuda_source_location {
    static consteval cuda_source_location current(
            const int line = __builtin_LINE(),
            const int column = __builtin_COLUMN(),
            const char* const file = __builtin_FILE()
    ) noexcept {
        cuda_source_location res{};
        res._Line     = line;
        res._Column   = column;
        res._File     = file;
        return res;
    }

    constexpr cuda_source_location() noexcept = default;

    __device__ constexpr int line() const noexcept {
        return _Line;
    }
    __device__ constexpr int column() const noexcept {
        return _Column;
    }
    __device__ constexpr const char* file_name() const noexcept {
        return _File;
    }

private:
    int _Line{};
    int _Column{};
    const char* _File     = "";
};

enum LoadHint{
    LoadHint_CA, // Cache at all levels, likely to be accessed again. (Default)
    LoadHint_CG, // Cache at global level (cache in L2 and below, not L1).
    LoadHint_CS, // Cache streaming, likely to be accessed once.
    LoadHint_LU, // last use (no write-back)
    LoadHint_CV, // Don’t cache and fetch again
};

enum StoreHint{
    StoreHint_WB, // Cache write-back all coherent levels.
    StoreHint_CG, // Cache at global level (cache in L2 and below, not L1).
    StoreHint_CS, // Cache streaming, likely to be accessed once.
    StoreHint_WT  // Cache write-through (to system memory).
};

const bool BOUNDS_CHECK = false;
const bool REPORT_OOB = false;

struct BoundsError{
    const char* file;
    int line;
    int index;
    int elements;
    int padding;
};

struct BoundsErrorArray{
    BoundsError* __restrict__ errors;
    int* __restrict__ count;
    int errors_size;
};

extern __constant__ BoundsErrorArray boundsErrorArray;

void CudaBufferSetupBoundsCheck();
void CudaBufferProcessBoundsCheckErrors();

template<typename T>
class CudaBuffer{
public:
    T* __restrict__ ptr;
    int numElements;
    bool owns_ptr;

    CudaBuffer() : ptr(nullptr), numElements(0), owns_ptr(false) {}

    __host__ __device__ CudaBuffer(T* ptr, int numElements, bool owns_ptr):
    ptr(ptr), numElements(numElements), owns_ptr(owns_ptr){

    }

    __host__ __device__ ~CudaBuffer(){
        #ifndef __CUDA_ARCH__
        if(owns_ptr){
            freeAsync();
        }
        #endif
    }

    __host__ __device__ CudaBuffer(const CudaBuffer& other) : ptr(other.ptr), numElements(other.numElements), owns_ptr(false) { }
    CudaBuffer& operator=(const CudaBuffer&) = delete;

    CudaBuffer(CudaBuffer&&) = delete;
    __host__ CudaBuffer& operator=(CudaBuffer&& other)  noexcept {
        if(this != &other) {
            if(this->owns_ptr && this->numElements != 0){
                freeAsync();
            }
            std::swap(ptr, other.ptr);
            std::swap(numElements, other.numElements);
            std::swap(owns_ptr, other.owns_ptr);
        }
        return *this;
    }

    __device__ void reportError(int index, const cuda_source_location location) const{
        int k = atomicAdd(boundsErrorArray.count, 1);
        if(k < boundsErrorArray.errors_size){
            boundsErrorArray.errors[k] = BoundsError{location.file_name(),
                                                     location.line(),
                                                     index,
                                                     numElements,
                                                     0};
        }
    }

    inline __device__ T load(int index, const cuda_source_location location = cuda_source_location::current()) const{
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }
        return ptr[index];
    }

    inline __device__ T load(int index, LoadHint hint, const cuda_source_location location = cuda_source_location::current()) const{
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }
        if(hint == LoadHint_CA){
            return __ldca(ptr+index);
        }else if(hint == LoadHint_CG){
            return __ldcg(ptr+index);
        }else if(hint == LoadHint_CS){
            return __ldcs(ptr+index);
        }else if(hint == LoadHint_LU){
            return __ldlu(ptr+index);
        }else if(hint == LoadHint_CV){
            return __ldcv(ptr+index);
        }else{
            return ptr[index];
        }
    }

    inline __device__ void store(int index, T value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
        }else{
            ptr[index] = value;
        }
    }

    inline __device__ void store(int index, T value, StoreHint hint, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
        }else{
            if(hint == StoreHint_WB){
                __stwb(ptr + index, value);
            }else if(hint == StoreHint_CG){
                __stcg(ptr + index, value);
            }else if(hint == StoreHint_CS){
                __stcs(ptr + index, value);
            }else if(hint == StoreHint_WT){
                __stwt(ptr + index, value);
            }else{
                ptr[index] = value;
            }
        }
    }

    inline __device__ T AtomicAdd(int index, T value, bool skipZero=false, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }else{
            if(skipZero && value == T()) return T();
            return ::atomicAdd(&ptr[index], value);
        }
    }

    inline __device__ void ReduceAdd(int index, T value, bool skipZero=false, const cuda_source_location location = cuda_source_location::current()){
        if constexpr (std::is_same_v<float, T>) {
            ReduceAddFloat(index, value, skipZero, location);
        } else if constexpr (std::is_same_v<Half4, T>) {
            ReduceAddHalf4(index, value, skipZero, location);
        } else {
            static_assert(false); // not implemented
        }
    }

    #define FLOAT_TO_CUI(var) *(reinterpret_cast<const uint*>(&(var)))
    inline __device__ void ReduceAddFloat(int index, float value, bool skipZero=false, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
        }else{
            if(skipZero && value == 0.0f) return;
            asm volatile("cvta.global.u64 %0, %0;\n red.relaxed.global.add.f32 [%0], %1;" :: "l"(ptr+index), "r"(FLOAT_TO_CUI(value)) : "memory");
        }
    }

    inline __device__ void ReduceAddHalf4(int index, Half4 value, bool skipZero=false, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
        }else{
            if(skipZero && value.isZero()) return;
            asm volatile("cvta.global.u64 %0, %0;\n red.relaxed.global.add.noftz.f16x2 [%0], %1;" :: "l"(((uint*)ptr)+2*index+0), "r"(value.data.x) : "memory");
            asm volatile("cvta.global.u64 %0, %0;\n red.relaxed.global.add.noftz.f16x2 [%0], %1;" :: "l"(((uint*)ptr)+2*index+1), "r"(value.data.y) : "memory");
        }
    }

    inline __device__ T AtomicMin(int index, T value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }else{
            return ::atomicMin(&ptr[index], value);
        }
    }

    inline __device__ float AtomicMinFloat(int index, float value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return float();
        }else{
            return atomicMinFloat(&ptr[index], value);
        }
    }

    inline __device__ T AtomicMax(int index, T value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }else{
            return atomicMax(&ptr[index], value);
        }
    }

    inline __device__ float AtomicMaxFloat(int index, float value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return float();
        }else{
            return atomicMaxFloat(&ptr[index], value);
        }
    }

    inline __device__ T AtomicExchange(int index, T value, const cuda_source_location location = cuda_source_location::current()){
        if(BOUNDS_CHECK && (index < 0 || index >= numElements)){
            if(REPORT_OOB)reportError(index, location);
            return T();
        }else{
            return atomicExch(&ptr[index], value);
        }
    }

    static CudaBuffer<T> fromGLBuffer(const GLBuffer& buff){
        if(!buff.getCudaPtr()){
            throw std::string("Cannot create CudaBuffer from nullptr");
        }
        if(buff.getSizeInBytes() % sizeof(T) != 0){
            throw std::string("Total byte size of GLBuffer must be a multiple of sizeof(T)");
        }
        return CudaBuffer<T>{(T* __restrict__)buff.getCudaPtr(), (int)(buff.getSizeInBytes() / sizeof(T)), false};
    }

    static CudaBuffer<T> allocate(int numElements, const std::string& name, cudaStream_t stream = 0){
        if(numElements <= 0){
            throw std::string("Error, trying to create CudaBuffer with numElements <= 0");
        }
        T* ptr;
        checkCudaErrors(cudaMallocAsync(&ptr, numElements * sizeof(T), stream));
        checkCudaErrors(cudaMemsetAsync(ptr, 0, numElements * sizeof(T), stream));
        CudaIntrospection::addBuffer(ptr, numElements * sizeof(T), name);
        return CudaBuffer<T>{ptr, numElements, true};
    }

    static CudaBuffer<T> allocate(const std::vector<T>& data, const std::string& name, cudaStream_t stream = 0){
        if(data.size() <= 0){
            throw std::string("Error, trying to create CudaBuffer with numElements <= 0");
        }
        T* ptr;
        checkCudaErrors(cudaMallocAsync(&ptr, data.size() * sizeof(T), stream));
        checkCudaErrors(cudaMemcpyAsync(ptr, data.data(), data.size() * sizeof(T), cudaMemcpyHostToDevice, stream));
        CudaIntrospection::addBuffer(ptr, data.size() * sizeof(T), name);
        return CudaBuffer<T>{ptr, int(data.size()), true};
    }

    static CudaBuffer<T> allocate(void* data_ptr, size_t count, const std::string& name, cudaStream_t stream = nullptr){
        if(count <= 0){
            throw std::string("Error, trying to create CudaBuffer with numElements <= 0");
        }
        if(count % sizeof(T) != 0){
            throw std::string("Error, trying to create CudaBuffer with count % sizeof(T) != 0");
        }
        T* ptr;
        checkCudaErrors(cudaMallocAsync(&ptr, count, stream));
        checkCudaErrors(cudaMemcpyAsync(ptr, data_ptr, count, cudaMemcpyHostToDevice, stream));
        CudaIntrospection::addBuffer(ptr, count, name);
        return CudaBuffer<T>{ptr, int(count / sizeof(T)), true};
    }

    void reallocate(int elements, const std::string& name, cudaStream_t stream = nullptr){
        *this = std::move(CudaBuffer<T>::allocate(elements, name, stream));
    }
    void reallocate(const std::vector<T>& data, const std::string& name, cudaStream_t stream = nullptr){
        *this = std::move(CudaBuffer<T>::allocate(data, name, stream));
    }
    void reallocate(void* data_ptr, size_t count, const std::string& name, cudaStream_t stream = nullptr){
        *this = std::move(CudaBuffer<T>::allocate(data_ptr, count, name, stream));
    }

    void freeAsync(cudaStream_t stream = nullptr){
        checkCudaErrors(cudaFreeAsync(ptr, stream));
        if(owns_ptr){
            CudaIntrospection::removeBuffer(ptr, numElements * sizeof(T));
        }
        ptr = nullptr;
        numElements = 0;
        owns_ptr = false;
    }

    void uploadAsync(const T* cpu_ptr, int countElements){
        checkCudaErrors(cudaMemcpyAsync((void*)ptr, (void*)cpu_ptr, countElements*sizeof(T), cudaMemcpyHostToDevice));
    }

    void downloadAsync(const T* cpu_ptr, int countElements){
        checkCudaErrors(cudaMemcpyAsync((void*)cpu_ptr, (void*)ptr, countElements*sizeof(T), cudaMemcpyDeviceToHost));
    }

    void zero(cudaStream_t stream = nullptr){
        checkCudaErrors(cudaMemsetAsync(ptr, 0, numElements*sizeof(T), stream));
    }

    void head(const std::string& title, int n, std::string (*printer)(T), const std::string& separator = " | "){
        if(n < 0) n = numElements;
        n = std::min(n, numElements);
        if(n == 0){
            std::cout <<"Empty CudaBuffer." <<std::endl;
            return;
        }
        std::vector<T> vec(n, T());
        checkCudaErrors(cudaMemcpy(vec.data(), ptr, n * sizeof(T), cudaMemcpyDeviceToHost));
        std::cout <<title <<", first " <<n <<" elements:" <<std::endl;
        for(int i=0; i<n; i++){
            std::cout <<printer(vec[i]) <<separator;
        }
        std::cout <<std::endl;
    }

    void head(std::string title="CudaBuffer", int n=10){
        head(title, n, [](T t){
            return std::to_string(t);
        });
    }


    void tail(std::string title, int n, std::string (*printer)(T), std::string separator = " | "){
        if(n < 0) n = numElements;
        n = std::min(n, numElements);
        if(n == 0){
            std::cout <<"Empty CudaBuffer." <<std::endl;
            return;
        }
        std::vector<T> vec(n, T());
        const int offset = numElements - n;
        checkCudaErrors(cudaMemcpy(vec.data(), ptr + offset, n * sizeof(T), cudaMemcpyDeviceToHost));
        std::cout <<title <<", last " <<n <<" elements:" <<std::endl;
        for(int i=0; i<n; i++){
            std::cout <<printer(vec[i]) <<separator;
        }
        std::cout <<std::endl;
    }

    void tail(std::string title="CudaBuffer", int n=10){
        tail(title, n, [](T t){
            return std::to_string(t);
        });
    }

    std::vector<T> cpu(int begin, int count){
        std::vector<T> vec(count, T());
        checkCudaErrors(cudaMemcpy(vec.data(), ptr+begin, count * sizeof(T), cudaMemcpyDeviceToHost));
        return vec;
    }

    std::vector<T> cpu(){
        return cpu(0, numElements);
    }

};

template<typename S, typename D>
inline __device__ CudaBuffer<D> convertCudaBuffer(CudaBuffer<S> src){
    // todo: size checks ?
    return CudaBuffer<D>{reinterpret_cast<D*>(src.ptr), (int)(src.numElements * sizeof(S) / sizeof(D)), false};
}

#endif //SPARSEVOXRECON_CUDABUFFER_CUH
