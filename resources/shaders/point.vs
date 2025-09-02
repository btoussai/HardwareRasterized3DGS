//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable

#include "./common/Uniforms.h"

___out vec4 baseColor;

void main(void){

    vec4 P = uniforms.positions[gl_VertexID];
    vec4 C = uniforms.predicted_colors[gl_VertexID];

    if(gl_VertexID == uniforms.selected_gaussian){
        C = vec4(1, 0, 1, 1);
        gl_PointSize = 10.0f;
    }else{
        gl_PointSize = 1.0f;
    }

    baseColor = C;
    gl_Position = uniforms.projMat * uniforms.viewMat * P;

}