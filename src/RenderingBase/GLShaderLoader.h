//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_GLSHADERLOADER_H
#define SPARSEVOXRECON_GLSHADERLOADER_H

#include <string>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#include "../glad/gl.h"
#include "Shader.h"

/**
 * Represents a header file (.h)
 */
struct ShaderHeader{
    std::string path; // path on disk
    std::string gl_path; // its glsl path, used for including.
    std::string contents; // the source itself
    uint64_t fileWriteTime; // time of last update of the file
    uint64_t cacheWriteTime; // time of last update of the cache
    std::unordered_set<std::string> direct_includes; // includes as written in contents
    std::unordered_set<std::string> recursive_includes; // direct and indirect includes
};

class GLShaderLoader {
public:
    GLShaderLoader(const std::string& shadersDirectory, const std::string& cacheDirectory);
    virtual ~GLShaderLoader();
    void deleteCache();

    void loadHeaders(const std::vector<std::string>& headerPaths,
                     const std::unordered_map<std::string, std::string>& replacements, const std::regex& re);

    static Shader load(const char *computeFilePath);
    static Shader load(const char* vertexFilePath, const char* fragmentFilePath);
    static Shader load(const char* vertexFilePath, const char* geometryFilePath, const char* fragmentFilePath);
    static Shader load(const char* vertexFilePath, const char* tessellationControlFilePath, const char* tessellationEvaluationFilePath, const char* geometryFilePath, const char* fragmentFilePath);
    static Shader load(const std::vector<std::string>& localPaths, std::vector<GLenum> types);

    void checkForFileUpdates();

    static GLShaderLoader* instance;
private:
    int loadingOrder = 0;

    std::string shadersDirectory;
    std::string cacheDirectory;

    std::regex re;
    std::unordered_map<std::string, std::string> replacements;
    std::regex replace_err_line_with_path = std::regex("0\\([0-9]+\\)");

    std::vector<std::string> headers_paths;
    std::unordered_map<std::string, ShaderHeader> headers;
    std::unordered_map<std::string, Shader*> shaders;

    ShaderHeader loadHeader(const std::string& path);
    ShaderSource loadSource(const std::string& path, const std::string& localPath, GLenum type, int loadingOrder);
    void writeCache(const ShaderSource& source);

    std::string loadFile(
            const std::string& path,
            std::unordered_set<std::string>& referenced_includes);
    void checkAndReplace(
            std::string &contents,
            std::string filePath,
            std::unordered_set<std::string>& referenced_includes);

    std::string replaceLineNumberWithSourcePath(std::string &msg, const std::string& source_path);
    bool compileSource(ShaderSource& source);

    static Shader loadAndCompileSources(const std::vector<std::string>& localPaths, std::vector<GLenum> types);
    void setListener(const std::string& path, Shader* shader);
    ShaderProgram loadShaderData(const std::vector<std::string>& localPaths, std::vector<GLenum> types, int loadingOrder);

    static void removeListener(Shader& shader);
};


#endif //SPARSEVOXRECON_GLSHADERLOADER_H
