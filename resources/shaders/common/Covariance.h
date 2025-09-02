//
// Created by Briac on 02/09/2025.
//

#ifndef HARDWARERASTERIZED3DGS_COVARIANCE_H
#define HARDWARERASTERIZED3DGS_COVARIANCE_H

#include "CommonTypes.h"

mat3 computeCov3D(const vec3 scale, float mod, const vec4 rot, const mat3 viewMat) {
    mat3 S = mat3(1.0f);
    S[0][0] = mod * scale.x;
    S[1][1] = mod * scale.y;
    S[2][2] = mod * scale.z;

    vec4 q = rot;
    float r = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    // Compute rotation matrix from quaternion
    mat3 R = mat3(
            1.f - 2.f * (y * y + z * z), 2.f * (x * y - r * z), 2.f * (x * z + r * y),
            2.f * (x * y + r * z), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - r * x),
            2.f * (x * z - r * y), 2.f * (y * z + r * x), 1.f - 2.f * (x * x + y * y)
    );

    const mat3 M = viewMat * transpose(R) * S;
    const mat3 Sigma = M * transpose(M);
    return Sigma;
}

vec2 computeAABB(const vec3 conic, const float opacity, const float min_alpha) {
    if(opacity < min_alpha){
        return vec2(0);
    }

    const float a = conic.x;
    const float b = conic.y;
    const float c = conic.z;

    const vec2 vx = vec2(1.0f, -b / c);
    const vec2 vy = vec2(-b / a, 1.0f);

    const float e = -2.0f * log(min_alpha / opacity);

    const float dx = sqrt(e / dot(vec3(vx.x*vx.x, 2.0f*vx.x*vx.y, vx.y*vx.y), conic));
    const float dy = sqrt(e / dot(vec3(vy.x*vy.x, 2.0f*vy.x*vy.y, vy.y*vy.y), conic));

    return ceil(vec2(dx, dy));
}

vec2 computeOBB(const vec3 conic, const float opacity, const float min_alpha, ___inout vec2 eigen_vec) {
    if(opacity < min_alpha){
        return vec2(0);
    }

    const float a = conic.x;
    const float b = conic.y;
    const float c = conic.z;

    const float half_tr = (a+c) * 0.5f;
    const float det = a*c - b*b;

    const float delta = sqrt(half_tr*half_tr - det);
    const float lambda1 = half_tr + delta;
    const float lambda2 = half_tr - delta;

    eigen_vec = normalize(vec2(-b, a - lambda1));
    const float e = -2.0f * log(min_alpha / opacity);

    const float dx = sqrt(e / lambda1);
    const float dy = sqrt(e / lambda2);

    return vec2(dx, dy);
}

vec3 computeCov2D(const vec3 mean, float focal_x, float focal_y, const mat3 cov3D) {
    const mat2x3 J = mat2x3(
            focal_x / mean.z, 0.0f, -(focal_x * mean.x) / (mean.z * mean.z),
            0.0f, focal_y / mean.z, -(focal_y * mean.y) / (mean.z * mean.z));
    const mat2 cov = transpose(J) * cov3D * J;
    return vec3( cov[0][0], cov[0][1], cov[1][1]);
}

//
//mat2 computeCov2D_Me(const vec3 mean, const mat3 cov3D, const vec3 ray){
//    const float z = mean.z;
//
//    const float a = dot(ray, cov3D * ray);
//    const mat2x3 Q = mat2x3(mat3(1.0f) - outerProduct(ray, ray) * cov3D * (1.0f / a));
//
//    const mat2x2 cov2D = transpose(Q) * cov3D * Q;
//    return cov2D * (z*z);
//}


#endif //HARDWARERASTERIZED3DGS_COVARIANCE_H
