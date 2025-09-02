//-- #version 460 core
//-- #extension GL_ARB_shading_language_include :   require
//-- #extension GL_NV_gpu_shader5 : enable
//-- #extension GL_NV_shader_buffer_load : enable
//-- #extension GL_ARB_bindless_texture : enable

#include "./common/GLSLDefines.h"

___out vec4 out_Color;

___in vec4 baseColor;

void main(void){

    out_Color = baseColor;

}