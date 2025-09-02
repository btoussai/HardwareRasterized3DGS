//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_KHR_shader_subgroup_clustered : enable

/*-- layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in; --*/

#include "./common/GLSLDefines.h"
#include "./common/Uniforms.h"

// Spherical harmonics coefficients
const float SH_C0 = 0.28209479177387814f;
const float SH_C1 = 0.4886025119029199f;
const float SH_C2[] = {
        1.0925484305920792f,
        -1.0925484305920792f,
        0.31539156525252005f,
        -1.0925484305920792f,
        0.5462742152960396f
};
const float SH_C3[] = {
        -0.5900435899266435f,
        2.890611442640554f,
        -0.4570457994644658f,
        0.3731763325901154f,
        -0.4570457994644658f,
        1.445305721320277f,
        -0.5900435899266435f
};

void main(void){
    const int n = int(gl_GlobalInvocationID.x) / 16;
    const int k = int(gl_GlobalInvocationID.x) % 16;
    if(n >= uniforms.num_gaussians)
        return;

    const vec4 P = uniforms.positions[n];

    const vec3 dir = normalize(vec3(P - uniforms.camera_pos));
    const float x = dir.x;
    const float y = dir.y;
    const float z = dir.z;
    const float xx = x * x, yy = y * y, zz = z * z;
    const float xy = x * y, yz = y * z, xz = x * z;

    const vec3 sh_coeff = vec3(
        uniforms.sh_coeffs_red[n * 16 + k],
        uniforms.sh_coeffs_green[n * 16 + k],
        uniforms.sh_coeffs_blue[n * 16 + k]
        );

    float weight = 0.0f;
    if(k== 0) weight = SH_C0;
    if(k== 1) weight = - SH_C1 * y;
    if(k== 2) weight = SH_C1 * z;
    if(k== 3) weight = - SH_C1 * x;
    if(k== 4) weight = SH_C2[0] * xy;
    if(k== 5) weight = SH_C2[1] * yz;
    if(k== 6) weight = SH_C2[2] * (2.0f * zz - xx - yy);
    if(k== 7) weight = SH_C2[3] * xz;
    if(k== 8) weight = SH_C2[4] * (xx - yy);
    if(k== 9) weight = SH_C3[0] * y * (3.0f * xx - yy);
    if(k==10) weight = SH_C3[1] * xy * z;
    if(k==11) weight = SH_C3[2] * y * (4.0f * zz - xx - yy);
    if(k==12) weight = SH_C3[3] * z * (2.0f * zz - 3.0f * xx - 3.0f * yy);
    if(k==13) weight = SH_C3[4] * x * (4.0f * zz - xx - yy);
    if(k==14) weight = SH_C3[5] * z * (xx - yy);
    if(k==15) weight = SH_C3[6] * x * (xx - 3.0f * yy);

    const vec3 result = max(0.5f + subgroupClusteredAdd(sh_coeff * weight, 16), 0.0f);

    if(k == 0){
        uniforms.predicted_colors[n] = vec4(result, 1.0f);
    }

}