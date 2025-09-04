//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable

#include "./common/Uniforms.h"

___flat ___out int InstanceID; // pass the index of the ellipse to the fragment shader
___out vec2 local_coord;

void main(void){

//    InstanceID = gl_InstanceID;
    InstanceID = gl_VertexID / 6;

    // Corners:
    // 2 3
    // 0 1

    // indices:  0  1  2  3  2  1
    // x:       -1 +1 -1 +1 -1 +1
    // y:       -1 -1 +1 +1 +1 -1

    float x = (gl_VertexID % 2 == 0) ? -1.0f : +1.0f;
    float y = (((gl_VertexID+1) % 6) < 3) ? -1.0f : + 1.0f;
    const vec2 corner = vec2(x, y);

    const float width = uniforms.width;
    const float height = uniforms.height;

    const vec4 box = uniforms.bounding_boxes[InstanceID]; // oriented bounding box
    const vec2 center = vec2(box.x, box.y); // bounding box center, in pixels
    const vec2 half_extent = vec2(box.z, box.w); // bounding box half size, in pixels

    // direction of major axis
    const vec2 dir = uniforms.eigen_vecs[InstanceID];

    // ellipse rotation matrix in 2D
    const mat2 rotation = mat2(dir.x, dir.y, -dir.y, dir.x);

    // offset of the corner of the oriented bounding box from the center of the 2D ellipse, in pixels
    local_coord = rotation * (half_extent * corner);

    // normalized coordinates
    const vec2 ndc = (center + local_coord) / vec2(width, height) * 2.0f - 1.0f;
    gl_Position = vec4(ndc, 0.0f, 1.0f);

}