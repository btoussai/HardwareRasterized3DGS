
#ifndef GLSLDEFINES_H_
#define GLSLDEFINES_H_

#ifdef __cplusplus
#define CPP

#include <inttypes.h>
#include "../../../src/glm/glm.hpp"
#include "../../../src/glm/gtc/packing.hpp"
#include "../../../src/glm/detail/type_half.hpp"

#include <cstring>
#include <array>


using namespace glm;

struct float16_t{
public:
	glm::detail::hdata f;

	float16_t(){
		this->f = glm::detail::toFloat16(0.0f);
	}

	float16_t(float f){
		this->f = glm::detail::toFloat16(f);
	}

	operator float() const{
		return glm::detail::toFloat32(f);

		vec4 v;
		v.operator *=(1.0f);
	}
};

typedef vec<2, float16_t, defaultp>		f16vec2;
typedef vec<3, float16_t, defaultp>		f16vec3;
typedef vec<4, float16_t, defaultp>		f16vec4;

//struct f16vec2{
//	glm::detail::hdata x, y;
//	f16vec2() : f16vec2(glm::vec2(0.0f)){
//	}
//	f16vec2(glm::vec2 v){
//		x = glm::detail::toFloat16(v.x);
//		y = glm::detail::toFloat16(v.y);
//	}
//	f16vec2(float a, float b){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(b);
//	}
//	f16vec2(float a){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(a);
//	}
//	operator glm::vec2() const{
//		return glm::vec2(glm::detail::toFloat32(x), glm::detail::toFloat32(y));
//	}
//};

//struct f16vec4{
//	glm::detail::hdata x, y, z, w;
//	f16vec4() : f16vec4(glm::vec4(0.0f)){
//
//	}
//	f16vec4(glm::vec4 v){
//		x = glm::detail::toFloat16(v.x);
//		y = glm::detail::toFloat16(v.y);
//		z = glm::detail::toFloat16(v.z);
//		w = glm::detail::toFloat16(v.w);
//	}
//	f16vec4(glm::vec3 v, float vw){
//		x = glm::detail::toFloat16(v.x);
//		y = glm::detail::toFloat16(v.y);
//		z = glm::detail::toFloat16(v.z);
//		w = glm::detail::toFloat16(vw);
//	}
//	f16vec4(float vx, glm::vec3 v){
//		x = glm::detail::toFloat16(vx);
//		y = glm::detail::toFloat16(v.x);
//		z = glm::detail::toFloat16(v.y);
//		w = glm::detail::toFloat16(v.z);
//	}
//	f16vec4(float a, float b, float c, float d){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(b);
//		z = glm::detail::toFloat16(c);
//		w = glm::detail::toFloat16(d);
//	}
//	f16vec4(float a){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(a);
//		z = glm::detail::toFloat16(a);
//		w = glm::detail::toFloat16(a);
//	}
//	operator glm::vec4() const{
//		return glm::vec4(glm::detail::toFloat32(x), glm::detail::toFloat32(y), glm::detail::toFloat32(z), glm::detail::toFloat32(w));
//	}
//};


//struct f16vec3{
//	glm::detail::hdata x, y, z;
//	f16vec3() : f16vec3(glm::vec3(0.0f)){
//
//	}
//	f16vec3(glm::vec3 v){
//		x = glm::detail::toFloat16(v.x);
//		y = glm::detail::toFloat16(v.y);
//		z = glm::detail::toFloat16(v.z);
//	}
//	f16vec3(glm::vec4 v){
//		x = glm::detail::toFloat16(v.x);
//		y = glm::detail::toFloat16(v.y);
//		z = glm::detail::toFloat16(v.z);
//	}
//	f16vec3(float a, float b, float c){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(b);
//		z = glm::detail::toFloat16(c);
//	}
//	f16vec3(float a){
//		x = glm::detail::toFloat16(a);
//		y = glm::detail::toFloat16(a);
//		z = glm::detail::toFloat16(a);
//	}
//	f16vec3(f16vec4 v){
//		x = v.x;
//		y = v.y;
//		z = v.z;
//	}
//	operator glm::vec3() const{
//		return glm::vec3(glm::detail::toFloat32(x), glm::detail::toFloat32(y), glm::detail::toFloat32(z));
//	}
//};

uint32_t packFloat2x16(f16vec2 v);

f16vec2 unpackFloat2x16(uint32_t v);

#define BACKWARD
#define rgba8
#define rgba16f
#define rgba32f
#define uniform
#define readonly
#define writeonly
#define restrict
#define shared
#define __UNKOWN_SIZE 1
#define ___discard

#define ___flat
#define ___out
#define ___in
#define ___inout

static const uint gl_VertexID = 0;
static const uint gl_InstanceID = 0;
static const uint gl_NumSubgroups = 0;
static const uint gl_SubgroupSize = 0;
static const uint gl_SubgroupID = 0;
static const uint gl_SubgroupInvocationID = 0;
static const uvec3 gl_WorkGroupID = uvec3(0);
static const uvec3 gl_WorkGroupSize = uvec3(0);
static const uvec3 gl_LocalInvocationID = uvec3(0);
static const uint gl_LocalInvocationIndex = 0;
static const uvec3 gl_GlobalInvocationID = uvec3(0);
static const uvec3 gl_NumWorkGroups = uvec3(0);
static const bool gl_FrontFacing = true;

static vec4 gl_Position = vec4(0);
static vec2 gl_PointCoord = vec2(0);

static const uint gl_WarpSizeNV = 32;
static const uint gl_SMCountNV = 84;
static const uint gl_WarpsPerSMNV = 48;
static const uint gl_WarpIDNV = 0;
static const uint gl_SMIDNV = 0;

static const struct gl_PerVertex
{
  vec4 gl_Position = vec4(0);
  float gl_PointSize = 0;
  float gl_ClipDistance[4] = {0, 0, 0, 0};
} gl_in[3];

// 1D images
struct image1D{
	image1D(uint64_t);
};
struct iimage1D{
	iimage1D(uint64_t);
};
struct uimage1D{
	uimage1D(uint64_t);
};
struct sampler1D{
	sampler1D(uint64_t);
};
struct isampler1D{
	isampler1D(uint64_t);
};
struct usampler1D{
	usampler1D(uint64_t);
};

// 2D images
struct image2D{
	image2D(uint64_t);
};
struct iimage2D{
	iimage2D(uint64_t);
};
struct uimage2D{
	uimage2D(uint64_t);
};
struct sampler2D{
	sampler2D(uint64_t);
};
struct isampler2D{
	isampler2D(uint64_t);
};
struct usampler2D{
	usampler2D(uint64_t);
};
struct sampler2DMS{
    sampler2DMS(uint64_t);
};

// 3D images
struct image3D{
	image3D(uint64_t);
};
struct iimage3D{
	iimage3D(uint64_t);
};
struct uimage3D{
	uimage3D(uint64_t);
};
struct sampler3D{
	sampler3D(uint64_t){

	}
};
struct isampler3D{
	isampler3D(uint64_t);
};
struct usampler3D{
	usampler3D(uint64_t);
};

// 2D arrays
struct sampler2DArray{
	sampler2DArray(uint64_t);
};
struct isampler2DArray{
	isampler2DArray(uint64_t);
};
struct usampler2DArray{
	usampler2DArray(uint64_t);
};
struct image2DArray{
	image2DArray(uint64_t);
};
struct iimage2DArray{
	iimage2DArray(uint64_t);
};
struct uimage2DArray{
	uimage2DArray(uint64_t);
};

// Cube images
struct imageCube{
	imageCube(uint64_t);
};
struct iimageCube{
	iimageCube(uint64_t);
};
struct uimageCube{
	uimageCube(uint64_t);
};
struct samplerCube{
	samplerCube(uint64_t);
};
struct isamplerCube{
	isamplerCube(uint64_t);
};
struct usamplerCube{
	usamplerCube(uint64_t);
};


vec4 imageLoad(image1D img, int c);
ivec4 imageLoad(iimage1D img, int c);
uvec4 imageLoad(uimage1D img, int c);
void imageStore(image1D img, int c, vec4 v);
void imageStore(iimage1D img, int c, ivec4 v);
void imageStore(uimage1D img, int c, uvec4 v);

vec4 imageLoad(image2D img, ivec2 c);
ivec4 imageLoad(iimage2D img, ivec2 c);
uvec4 imageLoad(uimage2D img, ivec2 c);
void imageStore(image2D img, ivec2 c, vec4 v);
void imageStore(iimage2D img, ivec2 c, ivec4 v);
void imageStore(uimage2D img, ivec2 c, uvec4 v);

vec4 imageLoad(image2DArray img, ivec3 c);
ivec4 imageLoad(iimage2DArray img, ivec3 c);
uvec4 imageLoad(uimage2DArray img, ivec3 c);
void imageStore(image2DArray img, ivec3 c, vec4 v);
void imageStore(iimage2DArray img, ivec3 c, ivec4 v);
void imageStore(uimage2DArray img, ivec3 c, uvec4 v);

vec4 imageLoad(image3D img, ivec3 c);
ivec4 imageLoad(iimage3D img, ivec3 c);
uvec4 imageLoad(uimage3D img, ivec3 c);
void imageStore(image3D img, ivec3 c, vec4 v);
void imageStore(iimage3D img, ivec3 c, ivec4 v);
void imageStore(uimage3D img, ivec3 c, uvec4 v);

vec4 imageLoad(imageCube img, ivec3 c);
ivec4 imageLoad(iimageCube img, ivec3 c);
uvec4 imageLoad(uimageCube img, ivec3 c);
void imageStore(imageCube img, ivec3 c, vec4 v);
void imageStore(iimageCube img, ivec3 c, ivec4 v);
void imageStore(uimageCube img, ivec3 c, uvec4 v);

f16vec4 imageAtomicAdd(image3D img, ivec3 c, f16vec4 v);
vec4 imageAtomicAdd(image2D img, ivec2 c, vec4 v);
uint imageAtomicAdd(iimage2D img, ivec2 c, uint v);


float imageAtomicAdd(image3D img, ivec3 c, float v);
int imageAtomicExchange(iimage2D img, ivec2 c, int v);
int imageAtomicCompSwap(iimage2D img, ivec2 c, int compare, int data);
int imageAtomicOr(iimage2D img, ivec2 c, int data);
int imageAtomicXor(iimage2D img, ivec2 c, int data);
int imageAtomicAnd(iimage2D img, ivec2 c, int data);

f16vec2 imageAtomicAdd(image3D img, ivec3 c, f16vec2 v);
int imageAtomicExchange(uimage2D img, ivec2 c, int v);
int imageAtomicCompSwap(uimage2D img, ivec2 c, int compare, int data);
int imageAtomicOr(uimage2D img, ivec2 c, int data);
int imageAtomicXor(uimage2D img, ivec2 c, int data);
int imageAtomicAnd(uimage2D img, ivec2 c, int data);

int imageAtomicExchange(iimage3D img, ivec3 c, int v);
int imageAtomicCompSwap(iimage3D img, ivec3 c, int compare, int data);
int imageAtomicOr(iimage3D img, ivec3 c, int data);
int imageAtomicXor(iimage3D img, ivec3 c, int data);
int imageAtomicAnd(iimage3D img, ivec3 c, int data);

uint imageAtomicExchange(uimage3D img, ivec3 c, uint v);
uint imageAtomicCompSwap(uimage3D img, ivec3 c, uint compare, uint data);
uint imageAtomicOr(uimage3D img, ivec3 c, uint data);
uint imageAtomicXor(uimage3D img, ivec3 c, uint data);
uint imageAtomicAnd(uimage3D img, ivec3 c, uint data);

vec4 texelFetch(sampler2DArray img, ivec3 c, int lod);
ivec4 texelFetch(isampler2DArray img, ivec3 c, int lod);
uvec4 texelFetch(usampler2DArray img, ivec3 c, int lod);

uvec4 texelFetch(usampler3D img, ivec3 c, int lod);
ivec4 texelFetch(isampler3D img, ivec3 c, int lod);
vec4 texelFetch(sampler1D img, int c, int lod);
vec4 texelFetch(sampler2D img, ivec2 c, int lod);
ivec4 texelFetch(isampler2D img, ivec2 c, int lod);
vec4 texelFetch(sampler3D img, ivec3 c, int lod);

vec4 texture(sampler1D img, float c);
vec4 texture(sampler2D img, vec2 c);
vec4 texture(sampler3D img, vec3 c);
vec4 texture(samplerCube img, vec3 c);
vec4 textureLod(samplerCube img, vec3 c, int lod);
vec4 textureGrad(sampler2D img, vec2 c, vec2 dpdx, vec2 dpdy);
vec4 textureLod(sampler3D img, vec3 c, int lod);
vec4 textureLod(sampler2D img, vec2 c, int lod);

vec4 texelFetch(sampler2DMS img, ivec2 c, int sample);

int imageSize(image1D);
int imageSize(iimage1D);
int imageSize(uimage1D);

ivec2 imageSize(image2D);
ivec2 imageSize(iimage2D);
ivec2 imageSize(uimage2D);

ivec3 imageSize(image2DArray);
ivec3 imageSize(iimage2DArray);
ivec3 imageSize(uimage2DArray);

ivec3 imageSize(image3D);
ivec3 imageSize(iimage3D);
ivec3 imageSize(uimage3D);

ivec2 imageSize(imageCube);
ivec2 imageSize(iimageCube);
ivec2 imageSize(uimageCube);

int textureSize(sampler1D, int);
int textureSize(isampler1D, int);
int textureSize(usampler1D, int);

ivec2 textureSize(sampler2D, int);
ivec2 textureSize(isampler2D, int);
ivec2 textureSize(usampler2D, int);

ivec3 textureSize(sampler2DArray, int);
ivec3 textureSize(isampler2DArray, int);
ivec3 textureSize(usampler2DArray, int);

ivec3 textureSize(sampler3D, int);
ivec3 textureSize(isampler3D, int);
ivec3 textureSize(usampler3D, int);

ivec2 textureSize(samplerCube, int);
ivec2 textureSize(isamplerCube, int);
ivec2 textureSize(usamplerCube, int);

vec4 texelFetchOffset(sampler2D sampler, ivec2 P, int lod, ivec2 offset);
vec4 textureGather(sampler2D, vec2, int comp);

void EmitVertex();
void EndPrimitive();

void barrier();
void subgroupBarrier();

uint64_t clockARB();

void memoryBarrier();
void memoryBarrierImage();
void memoryBarrierBuffer();
void memoryBarrierShared();
void subgroupMemoryBarrierShared();

bool subgroupElect();
bool subgroupAny(bool b);
bool subgroupAll(bool b);
int subgroupBroadcastFirst(int);
int subgroupBroadcast(int value, int id);
ivec2 subgroupBroadcast(ivec2 value, int id);
uvec4 subgroupBallot(bool);
uint subgroupBallotExclusiveBitCount(uvec4);
uint subgroupBallotInclusiveBitCount(uvec4);
uint subgroupBallotFindLSB(uvec4);
uvec4 subgroupPartitionNV(int);
vec3 subgroupPartitionedMinNV(vec3, uvec4);
vec3 subgroupPartitionedMaxNV(vec3, uvec4);
vec3 subgroupPartitionedAddNV(vec3, uvec4);
vec4 subgroupPartitionedAddNV(vec4, uvec4);

vec4 subgroupClusteredAdd(vec4, int);
vec3 subgroupClusteredAdd(vec3, int);
vec2 subgroupClusteredAdd(vec2, int);
float subgroupClusteredAdd(float, int);
int subgroupClusteredAdd(int, int);
f16vec4 subgroupClusteredAdd(f16vec4, int);

int subgroupClusteredOr(int, int);
int subgroupClusteredAnd(int, int);

int subgroupAdd(int v);
vec2 subgroupAdd(vec2 v);
vec3 subgroupAdd(vec3 v);
vec4 subgroupAdd(vec4 v);
ivec4 subgroupAdd(ivec4 v);
ivec3 subgroupAdd(ivec3 v);

uvec2 subgroupOr(uvec2 v);

vec4 subgroupMin(vec4 v);
ivec4 subgroupMin(ivec4 v);

vec2 subgroupMax(vec2 v);

float subgroupShuffleUp(float, uint);
float subgroupShuffleDown(float, uint);

vec4 inversesqrt(vec4);

int atomicAdd(int& mem, int data);
int atomicAdd(int* mem, int data);
float atomicAdd(float& mem, float data);
float atomicAdd(float* mem, float data);
double atomicAdd(double& mem, double data);
double atomicAdd(double* mem, double data);
f16vec2 atomicAdd(f16vec2* mem, f16vec2 data);
f16vec2 atomicAdd(f16vec2& mem, f16vec2 data);
f16vec4 atomicAdd(f16vec4* mem, f16vec4 data);
f16vec4 atomicAdd(f16vec4& mem, f16vec4 data);

uint atomicAdd(uint& mem, uint data);
uint atomicAnd(uint* mem, uint data);

uint atomicXor(uint& mem, uint data);
uint atomicXor(uint* mem, uint data);

int atomicMin(int& mem, int data);

uint64_t atomicOr(uint64_t* mem, uint64_t data);
uint64_t atomicAnd(uint64_t* mem, uint64_t data);

int atomicCompSwap(int& mem, int compare, int val);
int atomicCompSwap(int* mem, int compare, int val);
int atomicCompSwap(volatile int* mem, int compare, int val);

typedef uint atomic_uint;
uint atomicCounterAdd(atomic_uint, uint);
uint64_t atomicExchange(uint64_t mem, uint64_t data);

int subgroupExclusiveAdd(int);
ivec2 subgroupExclusiveAdd(ivec2);
ivec3 subgroupExclusiveAdd(ivec3);
ivec4 subgroupExclusiveAdd(ivec4);
uvec2 subgroupExclusiveAdd(uvec2);

int subgroupInclusiveAdd(int);

vec3 subgroupShuffleXor(vec3, uint);
vec4 subgroupShuffleXor(vec4, uint);
f16vec4 subgroupShuffleXor(f16vec4, uint);
uvec2 subgroupShuffleXor(uvec2, uint);

bool subgroupShuffle(bool, uint);
int subgroupShuffle(int, uint);
float subgroupShuffle(float, uint);
vec4 subgroupShuffle(vec4, uint);
uvec2 subgroupShuffle(uvec2, uint);

template<typename T>
T subgroupPartitionedOrNV(T, uvec4);

float dFdx(float);
vec2 dFdx(vec2);
float dFdy(float);
vec2 dFdy(vec2);
vec3 dFdx(vec3);
vec3 dFdy(vec3);

float dFdxFine(float);
float dFdyFine(float);
vec2 dFdxFine(vec2);
vec2 dFdyFine(vec2);

#endif


#endif /* GLSLDEFINES_H_ */
