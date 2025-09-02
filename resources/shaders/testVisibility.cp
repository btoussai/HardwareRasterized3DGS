//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_KHR_shader_subgroup_ballot : enable


const int NUM_WARPS = 128/32;
/*-- layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in; --*/

#include "./common/GLSLDefines.h"
#include "./common/Uniforms.h"
#include "./common/Covariance.h"

shared int warp_totals[NUM_WARPS];
shared int global_offset;

int prefixSum(bool ok){

    int local_value = (int)subgroupBallotExclusiveBitCount(subgroupBallot(ok));
    if(gl_SubgroupInvocationID == 31){
        warp_totals[gl_SubgroupID] = local_value + int(ok);
    }

    barrier();

    if(gl_LocalInvocationID.x == 0){
        int group_total = 0;
        for(int i=0; i<gl_NumSubgroups; i++){
            group_total += warp_totals[i];
        }

        global_offset = group_total > 0 ? atomicAdd(uniforms.visible_gaussians_counter, group_total) : 0;
    }

    barrier();

    for(int i=0; i<int(gl_SubgroupID); i++){
        local_value += warp_totals[i];
    }

    local_value += global_offset;

    return local_value;
}

void main(void){
    const int n = int(gl_GlobalInvocationID.x);
    bool ok = false;
    float depth = 0.0f;

    if(n < uniforms.num_gaussians) {
        const vec3 mean_world_space = vec3(uniforms.positions[n]);
        const vec3 scale = vec3(uniforms.scales[n]);
        const float opacity = uniforms.opacities[n];
        const vec4 quaternion = uniforms.rotations[n];

        const float scale_modifier = uniforms.scale_modifier;

        const float width = uniforms.width;
        const float height = uniforms.height;
        const float focal_x = uniforms.focal_x;
        const float focal_y = uniforms.focal_y;

        // transform to view space
        const vec3 mean = vec3(uniforms.viewMat * vec4(mean_world_space, 1.0f));

        const vec4 p_hom = uniforms.projMat * vec4(mean, 1.0f);
        const vec2 ndc = vec2(p_hom) / p_hom.w;
        depth = p_hom.w;

        const bool depth_ok = depth >= uniforms.near_plane && depth <= uniforms.far_plane;
        const bool selected = n == uniforms.selected_gaussian || uniforms.selected_gaussian == -1;
        const bool opacity_ok = opacity > uniforms.min_opacity;

        bool inSquare = false;
        inSquare = ndc.x > -2.0f && ndc.x < +2.0f && ndc.y > -2.0f && ndc.y < +2.0f;

        if(depth_ok && opacity_ok && inSquare) {
            const mat3 cov3D = computeCov3D(scale, scale_modifier, quaternion, mat3(uniforms.viewMat));
            vec3 cov = computeCov2D(mean, focal_x, focal_y, cov3D);

            const float h_var = 0.3f;
            const float det_cov = cov.x * cov.z - cov.y * cov.y;
            cov.x += h_var;
            cov.z += h_var;
            const float det_cov_plus_h_cov = cov.x * cov.z - cov.y * cov.y;
            float h_convolution_scaling = 1.0f;

            if(uniforms.antialiasing > 0)
                h_convolution_scaling = sqrt(max(0.000025f, det_cov / det_cov_plus_h_cov)); // max for numerical stability

            const float det = det_cov_plus_h_cov;

            if (det == 0.0f)
                return;

            const float det_inv = 1.0f / det;
            const vec3 conic = vec3( cov.z * det_inv, -cov.y * det_inv, cov.x * det_inv );

            const vec2 bbox_pixels = computeAABB(conic, opacity * h_convolution_scaling, uniforms.min_opacity);
            const vec2 proj_pixels = vec2(ndc * 0.5f + 0.5f) * vec2(width, height);


            const vec2 minCorner = proj_pixels - bbox_pixels;
            const vec2 maxCorner = proj_pixels + bbox_pixels;

            inSquare = maxCorner.x > 0.0f && minCorner.x < width && maxCorner.y > 0.0f && minCorner.y < height;
        }

        ok = depth_ok && inSquare && selected && opacity_ok;
    }

    const int index = prefixSum(ok);

    if(ok) {
        uniforms.gaussians_indices[index] = n;
        if(uniforms.front_to_back > 0){
            uniforms.gaussians_depth[index] = depth; // front to back
        }else{
            uniforms.gaussians_depth[index] = 1.0f / depth; // back to front
        }
    }


}