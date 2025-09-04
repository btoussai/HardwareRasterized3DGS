//
// Created by Briac on 02/09/2025.
//

#ifndef HARDWARERASTERIZED3DGS_COVARIANCE_H
#define HARDWARERASTERIZED3DGS_COVARIANCE_H

#include "CommonTypes.h"

mat3 quat2mat(const vec4 q){
    const float r = q.x;
    const float x = q.y;
    const float y = q.z;
    const float z = q.w;

    const mat3 R = mat3(1.0f) + 2.0f * mat3(
            -(y * y + z * z), (x * y - r * z), (x * z + r * y),
            (x * y + r * z), -(x * x + z * z), (y * z - r * x),
            (x * z - r * y), (y * z + r * x), -(x * x + y * y)
    );

    return R;
}

float mat3_sum(const mat3 m){
    return
    m[0][0] + m[0][1] + m[0][2] +
    m[1][0] + m[1][1] + m[1][2] +
    m[2][0] + m[2][1] + m[2][2];
}

void quat2mat_bwd(const vec4 q, const mat3 dLoss_dR, ___out vec4 dLoss_dq){
    const float r = q.x;
    const float x = q.y;
    const float y = q.z;
    const float z = q.w;

    const mat3 dR_dr = transpose(mat3(
            0.0f, z, -y,
            -z, 0.0f, x,
            y, -x, 0.0f));
    const mat3 dR_dx = transpose(mat3(
            0.0f, y, z,
            y, -2.0f*x, r,
            z, -r, -2.0f*x));
    const mat3 dR_dy = transpose(mat3(
            -2.0f*y, x, -r,
            x, 0, z,
            r, z, -2.0f*y));
    const mat3 dR_dz = transpose(mat3(
            -2.0f*z, r, x,
            -r, -2.0f*z, y,
            x, y, 0.0f));

    dLoss_dq.x = 2.0f * mat3_sum(matrixCompMult(dLoss_dR, dR_dr));
    dLoss_dq.y = 2.0f * mat3_sum(matrixCompMult(dLoss_dR, dR_dx));
    dLoss_dq.z = 2.0f * mat3_sum(matrixCompMult(dLoss_dR, dR_dy));
    dLoss_dq.w = 2.0f * mat3_sum(matrixCompMult(dLoss_dR, dR_dz));

    // normalize jacobian, assuming length(q) == 1.
    dLoss_dq = dLoss_dq - dot(q, dLoss_dq) * q;
}

mat3 computeCov3D(const vec3 scale, float mod, const vec4 q, const mat3 viewMat) {
    mat3 S = mat3(1.0f);
    S[0][0] = mod * scale.x;
    S[1][1] = mod * scale.y;
    S[2][2] = mod * scale.z;

    const mat3 R = quat2mat(q);
    const mat3 M = viewMat * transpose(R) * S;
    const mat3 Sigma = M * transpose(M);
    return Sigma;
}


void computeCov3D_bwd(const vec3 scale, const vec4 q, const mat3 viewMat,
                      const float dLoss_dcov3D[6], ___out vec3 dLoss_dscale, ___out vec4 dLoss_drot) {
    mat3 S = mat3(1.0f);
    S[0][0] = scale.x;
    S[1][1] = scale.y;
    S[2][2] = scale.z;

    const mat3 R = quat2mat(q);

    const mat3 Q = viewMat * transpose(R);
    const mat3 M = Q * S;
    const mat3 Sigma = M * transpose(M);

    const float a = Q[0][0];
    const float b = Q[1][0];
    const float c = Q[2][0];
    const float d = Q[0][1];
    const float e = Q[1][1];
    const float f = Q[2][1];
    const float g = Q[0][2];
    const float h = Q[1][2];
    const float i = Q[2][2];

    dLoss_dscale =
            2.0f * scale * (
        dLoss_dcov3D[0] * vec3(a*a, b*b, c*c) +
        dLoss_dcov3D[1] * vec3(a*d, e*b, c*f) +
        dLoss_dcov3D[2] * vec3(a*g, b*h, c*i) +
        dLoss_dcov3D[3] * vec3(d*d, e*e, f*f) +
        dLoss_dcov3D[4] * vec3(g*d, h*e, i*f) +
        dLoss_dcov3D[5] * vec3(g*g, h*h, i*i));

    const mat3 MS = M*S;

    mat3 dLoss_dR;

    quat2mat_bwd(q, dLoss_dR, dLoss_drot);

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

vec3 computeCov2D_simple(const vec3 mean, float focal, const mat3 cov3D) {
    const float invz = 1.0f / mean.z;
    const float h = focal * invz;
    const mat2x3 J = mat2x3(
            1.0f, 0.0f, -mean.x * invz,
            0.0f, 1.0f, -mean.y * invz);
    const mat2 cov = transpose(J) * cov3D * J;
    return vec3( cov[0][0], cov[0][1], cov[1][1]) * (h*h);
}

vec3 computeCov2D_simple2(const vec3 mean, float focal, const mat3 cov3D) {
    const float invz = 1.0f / mean.z;
    const float h = focal * invz;
    const float x = -mean.x * invz;
    const float y = -mean.y * invz;
    const float a = cov3D[0][0];
    const float b = cov3D[0][1];
    const float c = cov3D[0][2];
    const float d = cov3D[1][1];
    const float e = cov3D[1][2];
    const float f = cov3D[2][2];

    const float u = a + 2.0f * x * c  + x * x * f;
    const float v = b + x * e + y * c + x * y * f;
    const float w = d + 2.0f * y * e  + y * y * f;

    return vec3( u, v, w) * (h*h);
}

void computeCov2D_bwd(const vec3 mean, float focal, const mat3 cov3D,
                      const vec3 dLoss_dcov2d, ___out vec3 dLoss_dmean, ___out mat3 dLoss_dcov3D) {
    const float invz = 1.0f / mean.z;
    const float h = focal * invz;

    const float x = -mean.x * invz;
    const float y = -mean.y * invz;

    const float a = cov3D[0][0];
    const float b = cov3D[0][1];
    const float c = cov3D[0][2];
    const float d = cov3D[1][1];
    const float e = cov3D[1][2];
    const float f = cov3D[2][2];

    const float u = a + 2.0f * x * c  + x * x * f;
    const float v = b + x * e + y * c + x * y * f;
    const float w = d + 2.0f * y * e  + y * y * f;

    const vec3 cov2D = vec3( u, v, w) * (h*h);

    const float dLoss_dh = dot(dLoss_dcov2d, vec3( u, v, w)) * h * 2.0f;
    const vec3 dLoss_duvw = dLoss_dcov2d * (h*h);

    const float dLoss_dx = dLoss_duvw.x * (c + x*f) * 2.0f + dLoss_duvw.y * (e + y*f);
    const float dLoss_dy = dLoss_duvw.y * (c + x*f)        + dLoss_duvw.z * (e + y*f) * 2.0f;

    const float dLoss_dinvz = dLoss_dx * -mean.x + dLoss_dy * -mean.y + dLoss_dh * focal;
    dLoss_dmean = vec3(dLoss_dx, dLoss_dy, dLoss_dinvz * invz) * -invz;

    const float dLoss_da = dLoss_duvw.x;
    const float dLoss_db = dLoss_duvw.y;
    const float dLoss_dc = dLoss_duvw.x * x * 2.0f + dLoss_duvw.y * y;
    const float dLoss_dd = dLoss_duvw.z;
    const float dLoss_de = dLoss_duvw.y * x + dLoss_duvw.z * y * 2.0f;
    const float dLoss_df = dot(dLoss_duvw, vec3(x*x, x*y, y*y));

    dLoss_dcov3D = mat3(dLoss_da, dLoss_db, dLoss_dc,
                        dLoss_db, dLoss_dd, dLoss_de,
                        dLoss_dc, dLoss_de, dLoss_df);

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
