//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable
//-- #extension GL_ARB_fragment_shader_interlock : require

#include "./common/Uniforms.h"

___flat ___in int InstanceID;
___in vec2 local_coord; // offset of the corner of the oriented bounding box from the center of the 2D ellipse, in pixels

___in vec4 gl_FragCoord;

/*-- uniform layout(binding=0, rgba16f) restrict coherent --*/ image2D accumulated_image;

void main(void) {

    const vec4 color = uniforms.predicted_colors[InstanceID];
    const vec4 conic_opacity = uniforms.conic_opacity[InstanceID];

    const mat2 cov2D = mat2(conic_opacity.x, conic_opacity.y, conic_opacity.y, conic_opacity.z);
    const float opacity = conic_opacity.w;

    const float power = -0.5f * dot(local_coord, cov2D * local_coord);
    if (power > 0.0f){
        ___discard;
    }

    const float alpha = min(0.99f, opacity * exp(power));
    if (alpha < uniforms.min_opacity){
        ___discard;
    }

    const vec3 c = vec3(color);
    const ivec2 uv = ivec2(gl_FragCoord.x, gl_FragCoord.y);

    // critical section, manual alpha-blending
    beginInvocationInterlockARB();

    vec4 C = imageLoad(accumulated_image, uv);
    float transmittance = C.w;
    C = vec4(vec3(C) + c * transmittance * alpha, transmittance * (1.0f-alpha));
    imageStore(accumulated_image, uv, C);

    endInvocationInterlockARB();

}