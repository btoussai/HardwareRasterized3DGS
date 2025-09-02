//
// Created by Briac on 21/06/2025.
//

#ifndef SPARSEVOXRECON_CUDACOMMON_CUH
#define SPARSEVOXRECON_CUDACOMMON_CUH

#include <cuda.h>
#include <cuda_fp16.h>
#include "helper_math.h"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"

__device__ inline glm::ivec3 uncoil3D(int index, int S){
    glm::ivec3 p;
    p.x = index % S;
    p.y = (index / S) % S;
    p.z = index / (S * S);
    return p;
}

__device__ inline int coil3D(glm::ivec3 p, int S){
    return p.x + p.y * S + p.z * (S*S);
}

__host__ __device__ inline uint8_t packUnorm8(float f) {
    return (uint8_t) round(clamp(f, 0.0f, 1.0f) * 255.0f);
}
__host__ __device__ inline uint16_t packUnorm16(float f) {
    return (uint8_t) round(clamp(f, 0.0f, 1.0f) * 65535.0f);
}
__host__ __device__ inline int8_t packSnorm8(float f) {
    return (int8_t) round(clamp(f, -1.0f, 1.0f) * 127.0f);
}
__host__ __device__ inline int16_t packSnorm16(float f) {
    return (int8_t) round(clamp(f, -1.0f, 1.0f) * 32767.0f);
}
__host__ __device__ inline float unpackUnorm8(uint8_t x) {
    return float(x) / 255.0f;
}
__host__ __device__ inline float unpackUnorm16(uint16_t x) {
    return float(x) / 65535.0f;
}
__host__ __device__ inline float unpackSnorm8(int8_t f) {
    return clamp(float(f) / 127.0f, -1.0f, +1.0f);
}
__host__ __device__ inline float unpackSnorm16(int16_t f) {
    return clamp(float(f) / 32767.0f, -1.0f, +1.0f);
}


__device__ inline unsigned get_lane_id() {
    unsigned ret;
    asm volatile("mov.u32 %0, %laneid;" : "=r"(ret));
    return ret;
}

//https://stackoverflow.com/a/72461459
__device__ inline float atomicMinFloat(float* addr, float value) {
    float old;
    old = !signbit(value) ? __int_as_float(atomicMin((int*)addr, __float_as_int(value))) :
          __uint_as_float(atomicMax((unsigned int*)addr, __float_as_uint(value)));
    return old;
}

//https://stackoverflow.com/a/72461459
__device__ inline float atomicMaxFloat(float* addr, float value) {
    float old;
    old = !signbit(value) ? __int_as_float(atomicMax((int*)addr, __float_as_int(value))) :
          __uint_as_float(atomicMin((unsigned int*)addr, __float_as_uint(value)));
    return old;
}

template<typename T>
__host__ __device__ inline uint64_t type_as_uint64(T v) {
    return *reinterpret_cast<uint64_t*>(&v);
}

template<typename T>
__host__ __device__ inline T uint64_as_type(uint64_t v) {
    return *reinterpret_cast<T*>(&v);
}

template<typename T>
__host__ __device__ inline uint32_t type_as_uint32(T v) {
    return *reinterpret_cast<uint32_t*>(&v);
}

template<typename T>
__host__ __device__ inline T uint32_as_type(uint32_t v) {
    return *reinterpret_cast<T*>(&v);
}

__host__ __device__ inline __half2 uint_as_half2(uint v) {
    return *reinterpret_cast<__half2*>(&v);
}

__host__ __device__ inline uint half2_as_uint(__half2 v) {
    return *reinterpret_cast<uint*>(&v);
}

struct Half4{
    uint2 data;
    Half4() = default;
    __host__ __device__ inline explicit Half4(glm::vec4 v){
        __half2 a = __float22half2_rn(make_float2(v.x, v.y));
        __half2 b = __float22half2_rn(make_float2(v.z, v.w));
        data.x = half2_as_uint(a);
        data.y = half2_as_uint(b);
    }
    __host__ __device__ inline explicit Half4(float v){
        __half2 a = __float22half2_rn(make_float2(v, v));
        data.y = data.x = half2_as_uint(a);
    }
    __host__ __device__ inline explicit Half4(__half2 xy, __half2 zw){
        data.x = half2_as_uint(xy);
        data.y = half2_as_uint(zw);
    }
    __host__ __device__ inline glm::vec4 toVec4() const{
        __half2 a{}, b{};
        a = uint_as_half2(data.x);
        b = uint_as_half2(data.y);
        float2 c = __half22float2(a);
        float2 d = __half22float2(b);
        return {c.x, c.y, d.x, d.y};
    }

    __host__ __device__ inline bool isZero() const{
        return data.x == 0 && data.y == 0;
    }

    __device__ inline Half4& operator+=(const Half4& o){
        data.x = half2_as_uint(uint_as_half2(data.x) + uint_as_half2(o.data.x));
        data.y = half2_as_uint(uint_as_half2(data.y) + uint_as_half2(o.data.y));
        return *this;
    }
    __device__ inline Half4& operator-=(const Half4& o){
        data.x = half2_as_uint(uint_as_half2(data.x) - uint_as_half2(o.data.x));
        data.y = half2_as_uint(uint_as_half2(data.y) - uint_as_half2(o.data.y));
        return *this;
    }
    __device__ inline Half4& operator*=(const Half4& o){
        data.x = half2_as_uint(uint_as_half2(data.x) * uint_as_half2(o.data.x));
        data.y = half2_as_uint(uint_as_half2(data.y) * uint_as_half2(o.data.y));
        return *this;
    }
};

__device__ inline Half4 operator+(const Half4& a, const Half4& b){
    __half2 x = uint_as_half2(a.data.x) + uint_as_half2(b.data.x);
    __half2 y = uint_as_half2(a.data.y) + uint_as_half2(b.data.y);
    return Half4(x, y);
}
__device__ inline Half4 operator-(const Half4& a, const Half4& b){
    __half2 x = uint_as_half2(a.data.x) - uint_as_half2(b.data.x);
    __half2 y = uint_as_half2(a.data.y) - uint_as_half2(b.data.y);
    return Half4(x, y);
}
__device__ inline Half4 operator*(const Half4& a, const Half4& b){
    __half2 x = uint_as_half2(a.data.x) * uint_as_half2(b.data.x);
    __half2 y = uint_as_half2(a.data.y) * uint_as_half2(b.data.y);
    return Half4(x, y);
}

struct Half8{
    Half4 a, b;
};

template<typename T>
__device__ inline T warpPrefixSum_internal_u32(T value, bool exclusive=true){
    const uint32_t mask = 0xFFFFFFFF;
    const int lane = (int)get_lane_id();
    T sum = value;
    T n;
#pragma unroll
    for(int i=1; i<32; i*=2){
        n = uint32_as_type<T>(__shfl_up_sync(mask, type_as_uint32(sum), i));
        sum = (lane >=  i) ? sum + n : sum;
    }
    return exclusive ? sum - value : sum;
}

template<typename T>
__device__ inline T warpPrefixSum_internal_u64(T value, bool exclusive=true){
    const uint32_t mask = 0xFFFFFFFF;
    const int lane = (int)get_lane_id();
    T sum = value;
    T n;
#pragma unroll
    for(int i=1; i<32; i*=2){
        n = uint64_as_type<T>(__shfl_up_sync(mask, type_as_uint64(sum), i));
        sum = (lane >=  i) ? sum + n : sum;
    }
    return exclusive ? sum - value : sum;
}

template<typename T>
__device__ inline T warpPrefixSum(T value, bool exclusive=true){
    static_assert(!std::is_same<T, bool>::value); // disallow prefix sum on bool, must cast to int first.
    static_assert(sizeof(T) == 4 || sizeof(T) == 8);
    if(sizeof(T) == 4){
        value = warpPrefixSum_internal_u32<T>(value, exclusive);
    }else if(sizeof(T) == 8){
        value = warpPrefixSum_internal_u64<T>(value, exclusive);
    }
    return value;
}

template<typename T, int clusterSize=32>
__device__ inline T warpReduce_internal_u32(T value, uint32_t mask, T (*binaryOP)(T, T)){
    static_assert(clusterSize == 2 || clusterSize == 4 || clusterSize == 8 || clusterSize == 16 || clusterSize == 32);
#pragma unroll
    for(int i=1; i<clusterSize; i*=2){
        value = binaryOP(value, uint32_as_type<T>(__shfl_xor_sync(mask, type_as_uint32(value),  i, 32)));
    }
    return value;
}

template<typename T, int clusterSize=32>
__device__ inline T warpReduce_internal_u64(T value, uint32_t mask, T (*binaryOP)(T, T)){
    static_assert(clusterSize == 2 || clusterSize == 4 || clusterSize == 8 || clusterSize == 16 || clusterSize == 32);
    #pragma unroll
    for(int i=1; i<clusterSize; i*=2){
        value = binaryOP(value, uint64_as_type<T>(__shfl_xor_sync(mask, type_as_uint64(value),  i, 32)));
    }
    return value;
}

template<typename T, int clusterSize=32>
__device__ inline T warpReduce(T value, uint32_t mask, T (*binaryOP)(T, T)){
    static_assert(sizeof(T) == 4 || sizeof(T) == 8);
    if(sizeof(T) == 4){
        value = warpReduce_internal_u32<T, clusterSize>(value, mask, binaryOP);
    }else if(sizeof(T) == 8){
        value = warpReduce_internal_u64<T, clusterSize>(value, mask, binaryOP);
    }
    return value;
}

template<typename T, int clusterSize=32>
__device__ inline T warpAdd(T value, uint32_t mask=0xFFFFFFFF){
    return warpReduce<T, clusterSize>(value, mask, [](T a, T b){return a+b;});
}

template<typename T, int clusterSize=32>
__device__ inline T warpMul(T value, uint32_t mask=0xFFFFFFFF){
    return warpReduce<T, clusterSize>(value, mask, [](T a, T b){return a*b;});
}

template<typename T, int clusterSize=32>
__device__ inline T warpOr(T value, uint32_t mask=0xFFFFFFFF){
    return warpReduce<T, clusterSize>(value, mask, [](T a, T b){return a|b;});
}

template<int clusterSize=32>
__device__ inline float warpMin(float value, uint32_t mask=0xFFFFFFFF){
    return warpReduce<float, clusterSize>(value, mask, fminf);
}

template<int clusterSize=32>
__device__ inline float warpMax(float value, uint32_t mask=0xFFFFFFFF){
    return warpReduce<float, clusterSize>(value, mask, fmaxf);
}

#endif //SPARSEVOXRECON_CUDACOMMON_CUH
