//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable

#include "./common/Uniforms.h"

/*-- layout(location=0) in --*/ vec2 corner; // square with coords vec2(+/- 1.0f, +/- 1.0f)

___flat ___out int InstanceID; // pass the index of the ellipse to the fragment shader
___out vec2 local_coord;

void main(void){

    // framebuffer size
    const float width = uniforms.width;
    const float height = uniforms.height;

    const vec4 box = uniforms.bounding_boxes[gl_InstanceID]; // oriented bounding box
    const vec2 center = vec2(box.x, box.y); // bounding box center, in pixels
    const vec2 half_extent = vec2(box.z, box.w); // bounding box half size, in pixels

    // direction of major axis
    const vec2 dir = uniforms.eigen_vecs[gl_InstanceID];

    // ellipse rotation matrix in 2D
    const mat2 rotation = mat2(dir.x, dir.y, -dir.y, dir.x);

    // offset of the corner of the oriented bounding box from the center of the 2D ellipse, in pixels
    local_coord = rotation * (half_extent * corner);


    InstanceID = gl_InstanceID;
    // normalized coordinates
    const vec2 ndc = (center + local_coord) / vec2(width, height) * 2.0f - 1.0f;
    gl_Position = vec4(ndc, 0.0f, 1.0f);

}