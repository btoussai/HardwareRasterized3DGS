//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_SHADER_H
#define SPARSEVOXRECON_SHADER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "../glad/gl.h"
#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat3x3.hpp"
#include "../glm/mat4x4.hpp"

/**
 * Represents a glsl source file (.vs, .fs, .cp, etc...)
 */
struct ShaderSource{
    std::string path; // path on disk
    std::string localPath; // path relative to shadersDirectory
    GLuint ID; // GL object ID
    GLenum type; // vert / frag / compute, etc...
    std::string contents; // the source itself
    uint64_t fileWriteTime; // time of last update of the file
    std::unordered_set<std::string> direct_includes;
    std::unordered_set<std::string> recursive_includes; // direct and indirect includes

    int loadingOrder;
};

/**
 * Represents a compiled glsl program.
 */
struct ShaderProgram{
    GLuint programID;
    std::unordered_set<std::string> recursive_includes; // direct and indirect includes
    std::unordered_set<std::string> sources_paths;
    std::unordered_map<GLenum, ShaderSource> sources;
    int loadingOrder;
    bool compiledSuccessfully;
};

class Shader {
public:
    Shader();
    virtual ~Shader();

    void start() const;
    void stop() const;

    void bindVertexAttribute(GLuint attribute, const char* variableName);
    void bindFragDataLocation(GLuint colorAttachment, const char* variableName);
    void connectTextureUnit(const std::string& sampler_name, GLint value);

    void loadInt(const std::string& name, GLint value) const;
    void loadInt(const std::string& name, GLuint value) const;
    void loadFloat(const std::string& name, float value) const;
    void loadVec2(const std::string& name, glm::vec2 v) const;
    void loadiVec2(const std::string& name, glm::ivec2 v) const;
    void loaduVec2(const std::string& name, glm::uvec2 v) const;
    void loadVec3(const std::string& name, glm::vec3 v) const;
    void loadiVec3(const std::string& name, glm::ivec3 v) const;
    void loaduVec3(const std::string& name, glm::uvec3 v) const;
    void loadVec4(const std::string& name, glm::vec4 v) const;
    void loadiVec4(const std::string& name, glm::ivec4 v) const;
    void loaduVec4(const std::string& name, glm::uvec4 v) const;
    void loadMat3(const std::string& name, glm::mat3 mat) const;
    void loadMat4(const std::string& name, glm::mat4 mat) const;
    void loadUInt64(const std::string &name, GLuint64 value) const;
    void loadHandle(const std::string &name, GLuint64 value) const;

    GLint operator[](const std::string& name) {
        return uniforms[name];
    }

    void init_uniforms(const std::vector<std::string>& names);
    GLint getUniformLocation(const char* variableName) const;

    GLint findUniformLoc(const std::string& name) const;

    inline GLuint getProgramID() const {
        return data.programID;
    }

    ShaderProgram data;
    std::vector<std::string> uniforms_names;
    std::unordered_map<std::string, GLint> uniforms;
    std::function<void(Shader&)> deleter;
};


#endif //SPARSEVOXRECON_SHADER_H
