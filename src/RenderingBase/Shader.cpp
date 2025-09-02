//
// Created by Briac on 18/06/2025.
//

#include "Shader.h"

#include <iostream>

Shader::Shader() {

}

Shader::~Shader() {
    deleter(*this);
}

void Shader::start() const {
    glUseProgram(data.programID);
}

void Shader::stop() const {
    glUseProgram(0);
}

void Shader::bindVertexAttribute(GLuint attribute, const char *variableName) {
    glBindAttribLocation(data.programID, attribute, variableName);
}

void Shader::bindFragDataLocation(GLuint colorAttachment, const char *variableName) {
    glBindFragDataLocation(data.programID, colorAttachment, variableName);
}

void Shader::connectTextureUnit(const std::string &sampler_name, GLint value) {
    loadInt(sampler_name, value);
}

void Shader::loadInt(const std::string &name, GLint value) const {
    glUniform1i(findUniformLoc(name), value);
}

void Shader::loadInt(const std::string &name, GLuint value) const {
    glUniform1ui(findUniformLoc(name), value);
}

void Shader::loadFloat(const std::string &name, float value) const {
    glUniform1f(findUniformLoc(name), value);
}

void Shader::loadVec2(const std::string &name, glm::vec2 v) const {
    glUniform2f(findUniformLoc(name), v[0], v[1]);
}

void Shader::loadiVec2(const std::string &name, glm::ivec2 v) const {
    glUniform2i(findUniformLoc(name), v[0], v[1]);
}

void Shader::loaduVec2(const std::string &name, glm::uvec2 v) const {
    glUniform2ui(findUniformLoc(name), v[0], v[1]);
}

void Shader::loadVec3(const std::string &name, glm::vec3 v) const {
    glUniform3f(findUniformLoc(name), v[0], v[1], v[2]);
}

void Shader::loadiVec3(const std::string &name, glm::ivec3 v) const {
    glUniform3i(findUniformLoc(name), v[0], v[1], v[2]);
}

void Shader::loaduVec3(const std::string &name, glm::uvec3 v) const {
    glUniform3ui(findUniformLoc(name), v[0], v[1], v[2]);
}

void Shader::loadVec4(const std::string &name, glm::vec4 v) const {
    glUniform4f(findUniformLoc(name), v[0], v[1], v[2], v[3]);
}

void Shader::loadiVec4(const std::string &name, glm::ivec4 v) const {
    glUniform4i(findUniformLoc(name), v[0], v[1], v[2], v[3]);
}

void Shader::loaduVec4(const std::string &name, glm::uvec4 v) const {
    glUniform4ui(findUniformLoc(name), v[0], v[1], v[2], v[3]);
}

void Shader::loadMat3(const std::string& name, glm::mat3 mat) const{
    static float buffer[9];

    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            buffer[3 * x + y] = mat[x][y];
        }
    }

    glUniformMatrix3fv(findUniformLoc(name), 1,
                       GL_FALSE, (const GLfloat*) &buffer);
}
void Shader::loadMat4(const std::string &name, glm::mat4x4 mat) const {
    static float buffer[16];

    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            buffer[4 * x + y] = mat[x][y];
        }
    }

    glUniformMatrix4fv(findUniformLoc(name), 1,
                       GL_FALSE, (const GLfloat*) &buffer);
}

void Shader::loadUInt64(const std::string &name, GLuint64 value) const {
    glUniform1ui64ARB(findUniformLoc(name), value);
}

void Shader::loadHandle(const std::string &name, GLuint64 value) const {
    glUniformHandleui64ARB(findUniformLoc(name), value);
}

void Shader::init_uniforms(const std::vector<std::string> &names) {
    start();
    for (const std::string &name : names) {
        GLint loc = getUniformLocation(name.c_str());
        if (loc == -1) {
            std::cout << "Uniform location of " << name << " = " << loc << std::endl;
            std::cout << " 	--> The uniform variable name is either incorrect or the uniform variable is not used" << std::endl;
            uniforms[name] = loc;
        } else {
            uniforms[name] = loc;
        }
    }
    stop();
    uniforms_names = names;
}

GLint Shader::getUniformLocation(const char *variableName) const {
    return glGetUniformLocation(data.programID, variableName);
}

GLint Shader::findUniformLoc(const std::string &name) const {
    const auto& it = uniforms.find(name);
    if (it == end(uniforms)) {
        throw std::string("Error, unknown uniform variable name: ") + std::string(name);
    }
    return it->second;
}
