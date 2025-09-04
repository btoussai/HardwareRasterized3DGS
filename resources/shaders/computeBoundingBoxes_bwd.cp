//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_KHR_shader_subgroup_ballot : enable


/*-- layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in; --*/

#include "./common/GLSLDefines.h"
#include "./common/Uniforms.h"
#include "./common/Covariance.h"

void main(void){
    const int n = int(gl_GlobalInvocationID.x);
    if(n >= *uniforms.visible_gaussians_counter)
        return;

    const int GaussianID = uniforms.sorted_gaussian_indices[n];

    const vec4 dLoss_dconic_opacity = vec4(uniforms.dLoss_dconic_opacity[n]);

    const float opacity = uniforms.opacities[GaussianID];
    const vec3 mean_world_space = vec3(uniforms.positions[GaussianID]);
    const vec3 scale = vec3(uniforms.scales[GaussianID]);
    const vec4 quaternion = uniforms.rotations[GaussianID];

    const float width = uniforms.width;
    const float height = uniforms.height;
    const float focal_x = uniforms.focal_x;
    const float focal_y = uniforms.focal_y;

    // transform to view space
    const vec3 mean = vec3(uniforms.viewMat * vec4(mean_world_space, 1.0f));
    const mat3 cov3D = computeCov3D(scale, 1.0f, quaternion, mat3(uniforms.viewMat));

    const vec4 p_hom = uniforms.projMat * vec4(mean, 1.0f);
    const vec2 ndc = vec2(p_hom) / p_hom.w;

    // Compute 2D screen-space covariance matrix
    vec3 cov = computeCov2D(mean, focal_x, focal_y, cov3D);

    const float h_var = 0.3f;
    cov.x += h_var;
    cov.z += h_var;
    const float det_cov_plus_h_cov = cov.x * cov.z - cov.y * cov.y;
    const float det = det_cov_plus_h_cov;

    if (det == 0.0f)
        return;

    const float det_inv = 1.0f / det;
    const vec3 conic = vec3( cov.z * det_inv, -cov.y * det_inv, cov.x * det_inv );

    const vec3 dLoss_dconic = vec3(dLoss_dconic_opacity);
    const float dLoss_ddet_inv = dot(dLoss_dconic, vec3(cov.z, -cov.y, cov.x));


}