//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_ARB_fragment_shader_interlock : require
//-- #extension GL_NV_shader_subgroup_partitioned: enable
//-- #extension GL_KHR_shader_subgroup_ballot: enable
//-- #extension GL_KHR_shader_subgroup_arithmetic: enable
//-- #extension GL_NV_shader_atomic_float: enable
//-- #extension GL_NV_shader_atomic_fp16_vector: enable

#include "./common/Uniforms.h"

___flat ___in int InstanceID;
___in vec2 local_coord; // offset of the corner of the oriented bounding box from the center of the 2D ellipse, in pixels

___in vec4 gl_FragCoord;

/*-- uniform layout(binding=0, rgba16f) restrict coherent --*/ image2D accumulated_image;

void main(void) {

    if(gl_HelperInvocation){
        ___discard;
    }

    const ivec2 uv = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    const vec3 finalColor = vec3(imageLoad(/*--layout(rgba16f) restrict --*/ image2D(uniforms.accumulated_image_fwd), uv));
    const vec3 gtColor = vec3(imageLoad(/*--layout(rgba8) restrict --*/ image2D(uniforms.ground_truth_image), uv));

    // Loss = sum(dot(finalColor - gtColor, finalColor - gtColor))

    const vec3 dLoss_dFinalColor = 2.0f * (finalColor - gtColor);

    const vec3 color = vec3(uniforms.predicted_colors[InstanceID]);
    const vec4 conic_opacity = uniforms.conic_opacity[InstanceID];

    const mat2 cov2D = mat2(conic_opacity.x, conic_opacity.y, conic_opacity.y, conic_opacity.z);
    const float opacity = conic_opacity.w;

    const float power = -0.5f * dot(local_coord, cov2D * local_coord);
    if (power > 0.0f){
        ___discard;
    }

    const float expPower = exp(power);

    const float alpha = min(0.99f, opacity * exp(power));
    if (alpha < uniforms.min_opacity){
        ___discard;
    }

    float partial_transmittance = 0.0f;
    vec3 partial_color = vec3(0.0f);

    // critical section, manual alpha-blending
    beginInvocationInterlockARB();
    const vec4 C = imageLoad(accumulated_image, uv);
    partial_transmittance = C.w;
    partial_color = vec3(C) + color * partial_transmittance * alpha;
    imageStore(accumulated_image, uv, vec4(partial_color, partial_transmittance * (1.0f-alpha)));
    endInvocationInterlockARB();

    const float dLoss_dalpha = dot(dLoss_dFinalColor, color) * partial_transmittance - dot(dLoss_dFinalColor,finalColor - partial_color) / (1.0f - alpha);
    vec3 dLoss_dcolor = dLoss_dFinalColor * partial_transmittance * alpha;

    const float dLoss_dopacity = dLoss_dalpha * expPower;
    const float dLoss_dpower = dLoss_dalpha * alpha;
    const vec3 dLoss_dconic = -0.5f * dLoss_dpower * vec3(local_coord.x*local_coord.x, 2.0f*local_coord.x*local_coord.y, local_coord.y*local_coord.y);

    vec4 dLoss_dconic_opacity = alpha < 0.99f ? vec4(dLoss_dconic, dLoss_dopacity) : vec4(0.0f);

    const int GaussianID = uniforms.sorted_gaussian_indices[InstanceID];
    const uvec4 ballot = subgroupPartitionNV(InstanceID);
    const bool firstInPartition = subgroupBallotFindLSB(ballot) == gl_SubgroupInvocationID;

    dLoss_dcolor = subgroupPartitionedAddNV(dLoss_dcolor, ballot);
    dLoss_dconic_opacity = subgroupPartitionedAddNV(dLoss_dconic_opacity, ballot);

    if(firstInPartition){
        atomicAdd(uniforms.dLoss_dpredicted_colors+InstanceID, f16vec4(vec4(dLoss_dcolor, 0.0f)));
        atomicAdd(uniforms.dLoss_dconic_opacity+InstanceID, f16vec4(dLoss_dconic_opacity));
    }

}