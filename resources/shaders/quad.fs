//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable

#include "./common/Uniforms.h"

___out vec4 out_Color;

___flat ___in int InstanceID;
___in vec2 local_coord; // offset of the corner of the oriented bounding box from the center of the 2D ellipse, in pixels

void main(void){

    const vec4 color = uniforms.predicted_colors[InstanceID];
    const vec4 conic_opacity = uniforms.conic_opacity[InstanceID];

    const mat2 cov2D = mat2(conic_opacity.x, conic_opacity.y, conic_opacity.y, conic_opacity.z);
    const float opacity = conic_opacity.w;

    const float power = -0.5f * dot(local_coord, cov2D * local_coord);
    if (power > 0.0f){
        out_Color = vec4(0, 1, 1, 1);
        //return;
        ___discard;
    }

    float alpha = min(0.99f, opacity * exp(power));
    if (alpha < uniforms.min_opacity){
        out_Color = vec4(1, 0, 1, 1);
        //return;
        ___discard;
    }

    out_Color = vec4(vec3(color) * alpha, alpha);
}