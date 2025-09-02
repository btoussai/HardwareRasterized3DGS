//
// Created by Briac on 18/06/2025.
//

#include "GLShaderLoader.h"

#include <iostream>
#include <fstream>

const bool Silent = false;
GLShaderLoader* GLShaderLoader::instance = nullptr;

GLShaderLoader::GLShaderLoader(const std::string &shadersDirectory, const std::string &cacheDirectory) :
    shadersDirectory(shadersDirectory), cacheDirectory(cacheDirectory)
{
    if(GLShaderLoader::instance != nullptr){
        throw std::string("Error, a ShaderLoader instance has already been created");
    }
    GLShaderLoader::instance = this;
}

GLShaderLoader::~GLShaderLoader() {

}

void GLShaderLoader::deleteCache() {
    std::filesystem::path cachePath = std::filesystem::absolute(std::filesystem::path("shaderCache"));
    std::cout <<"Deleting shader cache: " <<cachePath <<std::endl;
    std::filesystem::remove_all(cachePath);
}

void GLShaderLoader::loadHeaders(const std::vector<std::string> &headerPaths,
                                 const std::unordered_map<std::string, std::string> &replacements,
                                 const std::regex &re) {

    this->re = re;
    this->replacements = replacements;
    this->headers_paths.clear();
    this->headers.clear();
    this->shaders.clear();

    for(std::string path : headerPaths){
        std::filesystem::path p = std::filesystem::path(path);
        path = std::filesystem::absolute(p).string();
        std::replace(path.begin(), path.end(), '\\', '/');

        this->headers_paths.push_back(path);
        this->headers[path] = loadHeader(path);
    }
}

ShaderHeader GLShaderLoader::loadHeader(const std::string &path) {
    ShaderHeader header;
    header.path = path;

    if(!Silent) std::cout <<"Loading GL header " <<path <<std::endl;

    header.contents = loadFile(path, header.direct_includes);

    // make sure the path starts at the root
    header.gl_path = path[0] != '/' ? "/" + path : path;
    if(glIsNamedStringARB(-1, header.gl_path.c_str())){
        glDeleteNamedStringARB(-1, header.gl_path.c_str());
    }
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, header.gl_path.c_str(), -1, header.contents.c_str());

    std::string includeName = std::filesystem::path(path).filename().string();
    std::filesystem::path cacheLoc = std::filesystem::absolute("shaderCache/" + this->cacheDirectory + "/common/" + includeName);

    header.fileWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();
    if(std::filesystem::exists(cacheLoc)){
        header.cacheWriteTime = std::filesystem::last_write_time(cacheLoc).time_since_epoch().count();
    }else{
        header.cacheWriteTime = 0;
    }

    bool cached = true;
    if(header.fileWriteTime > header.cacheWriteTime){
        std::filesystem::create_directories(cacheLoc.parent_path());
        std::ofstream output(cacheLoc.c_str());
        output <<header.contents;
        output.close();
        header.cacheWriteTime = std::filesystem::last_write_time(cacheLoc).time_since_epoch().count();
        cached = false;
    }

    for(const std::string& h : header.direct_includes){
        const auto& r = this->headers[h].recursive_includes;
        header.recursive_includes.insert(h);
        header.recursive_includes.insert(r.begin(), r.end());
    }

    if(!Silent){
        if(!cached){
            std::cout <<path <<" is out of sync with the cache." <<std::endl;
        }else{
            std::cout <<path <<" hasn't been modified." <<std::endl;
        }
    }

    return header;
}

std::string GLShaderLoader::loadFile(const std::string &path, std::unordered_set<std::string> &referenced_includes) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cout << "Couldn't find " << path << std::endl;
        std::cout << "Last dir searched: " << std::filesystem::current_path() << "/" << path << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string contents;
    {
        std::stringstream ss;
        std::string line;
        while(std::getline(f, line)){
            ss << line <<"\n";
        }
        contents = ss.str();
    }
    if(!path.ends_with("GLSLDefines.h")){ // GLSLDefines.h is a special case
        checkAndReplace(contents, path, referenced_includes);
    }

    return contents;
}

void GLShaderLoader::checkAndReplace(std::string &contents, std::string filePath,
                                     std::unordered_set<std::string> &referenced_includes) {

    std::string dir = filePath.substr(0, filePath.find_last_of("\\/"));

    auto words_begin = std::sregex_iterator(contents.begin(), contents.end(), re);
    auto words_end = std::sregex_iterator();

    std::string result;
    result.reserve(contents.size() * 2);

    int last_pos = 0;

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();

        int m_begin = (int)match.position(0);
        int m_length = (int)match.length(0);
        result += contents.substr(last_pos, m_begin - last_pos);

        if(match_str == "#include"){

            auto start = contents.find('"', m_begin + m_length);
            auto end = contents.find('"', start+1);

            if(start == contents.npos || end == contents.npos){
                // bad include
                auto line_end = contents.find('\n', m_begin + m_length);

                std::cout <<"Skipped malformed include: " <<contents.substr(m_begin, line_end - m_begin) <<std::endl;
                std::cout <<"Only relative includes with quotes are accepted." <<std::endl;
                std::cout <<"In file: " <<filePath <<std::endl;

                last_pos = line_end + 1; // Remove the faulty include
            }else{
                last_pos = end + 1;

                std::string path = contents.substr(start+1, end-start-1);
                std::filesystem::path p = std::filesystem::path(path);

                if(p.is_relative()){
                    path = dir + "/" + path;
                    p = std::filesystem::path(path);
                }
                try {
                    path = std::filesystem::canonical(p).string();
                } catch(const std::exception& ex) {
                    std::cout << "Canonical path for " << p << " threw exception:\n"
                              << ex.what()
                              <<"\nwhile processing " <<filePath << std::endl;
                    exit(-1);
                }
                std::replace(path.begin(), path.end(), '\\', '/');

                if(this->headers.find(path) == this->headers.end()){
                    throw "Error while including " + path + " by " + filePath + ", included file is not yet defined.";
                }

                if(path[0] == '/'){
                    result += "#include \"" + path + "\"";
                }else{
                    result += "#include \"/" + path + "\""; //windows
                }

                referenced_includes.insert(path);
            }

        }else{
            result += replacements[match_str];
            last_pos = m_begin + m_length;
        }

    }

    result += contents.substr(last_pos, std::string::npos);
    contents = result;

    if(false){
        //output the preprocessed shader
        std::string path = std::regex_replace(filePath, std::regex("resources/shaders"), "preprocessor");
        std::string dirpath = path.substr(0, path.find_last_of("\\/"));
        std::filesystem::create_directories(dirpath);
        std::ofstream output(path);
        output <<contents;
    }

}

ShaderSource GLShaderLoader::loadSource(const std::string &path, const std::string &localPath, GLenum type,
                                        int loadingOrder) {
    ShaderSource source;
    source.path = path;
    source.localPath = localPath;
    source.contents = loadFile(path, source.direct_includes);
    source.type = type;

    for(const std::string& h : source.direct_includes){
        const auto& r = this->headers[h].recursive_includes;
        source.recursive_includes.insert(h);
        source.recursive_includes.insert(r.begin(), r.end());
    }

    source.fileWriteTime = std::filesystem::last_write_time(path).time_since_epoch().count();
    source.loadingOrder = loadingOrder;

    return source;
}

void GLShaderLoader::writeCache(const ShaderSource &source) {
    std::filesystem::path cacheLoc = std::filesystem::absolute("shaderCache/" + this->cacheDirectory + "/" +
                                                               std::to_string(source.loadingOrder) + "/" + source.localPath);
    std::filesystem::create_directories(cacheLoc.parent_path());
    std::ofstream output(cacheLoc.c_str());
    output <<source.contents;
    output.close();
}

bool GLShaderLoader::compileSource(ShaderSource &source) {

    std::cout << "Compiling " << source.path << std::endl;

    source.ID = glCreateShader(source.type);
    const char *c_str = source.contents.c_str();
    int length = source.contents.length();
    glShaderSource(source.ID, 1, &c_str, &length);

    std::vector<const char*> strings;
    strings.reserve(headers_paths.size());
    for (int i = 0; i < (int)headers_paths.size(); ++i){
        strings.push_back(headers.find(headers_paths[i])->second.gl_path.c_str());
    }

    glCompileShaderIncludeARB(source.ID, this->headers_paths.size(), strings.data(), nullptr);

    GLint status = 0;
    glGetShaderiv(source.ID, GL_COMPILE_STATUS, &status);
    GLint sizeNeeded = 0;
    glGetShaderiv(source.ID, GL_INFO_LOG_LENGTH, &sizeNeeded);

    if(sizeNeeded > 0 || status == GL_FALSE) {
        std::string log_msg(sizeNeeded, ' ');
        glGetShaderInfoLog(source.ID, sizeNeeded, NULL, log_msg.data());

        // Scan the log message to replace 0(__LINE__) with file_path(__LINE__)
        log_msg = replaceLineNumberWithSourcePath(log_msg, source.path);

        if (status == GL_FALSE){
            std::cout << "Error while compiling " << source.path << " :" << std::endl;
        }else{
            std::cout << "Warning while compiling " << source.path << " :" << std::endl;
        }
        std::cout << log_msg << std::endl;

        return status == GL_TRUE;
    }

    return true;
}

Shader GLShaderLoader::loadAndCompileSources(const std::vector<std::string> &localPaths, std::vector<GLenum> types) {
    Shader shader;

    GLShaderLoader& loader = *GLShaderLoader::instance;

    shader.data = loader.loadShaderData(localPaths, types, loader.loadingOrder);
    loader.loadingOrder++;

    if(!shader.data.compiledSuccessfully){
        std::cout <<"Shader compile error" <<std::endl;
        exit(-1);
    }else{
        for(auto& it : shader.data.sources){
            loader.setListener(it.second.path, &shader);
        }
    }

    return shader;
}

void GLShaderLoader::setListener(const std::string &path, Shader *shader) {
    if(shader == nullptr){
        shaders.erase(shaders.find(path));
    }else{
        shaders[path] = shader;
    }
}

ShaderProgram GLShaderLoader::loadShaderData(const std::vector<std::string> &localPaths, std::vector<GLenum> types,
                                          int loadingOrder) {
    assert(localPaths.size() == types.size());
    ShaderProgram data;

    for(int i=0; i<(int)localPaths.size(); i++){
        std::filesystem::path p = std::filesystem::path(std::string("resources/shaders/") + localPaths[i]);
        std::string path = std::filesystem::absolute(p).string();
        std::replace(path.begin(), path.end(), '\\', '/');

        ShaderSource source = loadSource(path, localPaths[i], types[i], loadingOrder);
        data.loadingOrder = loadingOrder;
        data.recursive_includes.insert(source.recursive_includes.begin(), source.recursive_includes.end());
        data.sources_paths.insert(source.path);
        data.sources[types[i]] = std::move(source);
    }

    data.programID = glCreateProgram();
    const auto getLog = [&data, this](const std::string file_path){
        GLint sizeNeeded = 0;
        glGetProgramiv(data.programID, GL_INFO_LOG_LENGTH, &sizeNeeded);
        if(sizeNeeded > 0){
            std::vector<char> buff(sizeNeeded);
            glGetProgramInfoLog(data.programID, sizeNeeded, NULL, buff.data());
            std::string log_msg = std::string(buff.begin(), buff.end());

            // Scan the log message to replace 0(__LINE__) with file_path(__LINE__)
            log_msg = replaceLineNumberWithSourcePath(log_msg, file_path);
            std::cout << log_msg << std::endl;
        }
    };

    std::filesystem::path cacheLoc = std::filesystem::absolute("shaderCache/" + this->cacheDirectory + "/" +
                                                               std::to_string(loadingOrder) + "/cache.bin");

    if(std::filesystem::exists(cacheLoc)){
        bool cached = true;
        const uint64_t cacheWriteTime = std::filesystem::last_write_time(cacheLoc).time_since_epoch().count();
        for(const auto& source : data.sources){
            if(source.second.fileWriteTime > cacheWriteTime){
                cached = false;
                break;
            }
        }
        if(cached){
            for(const auto& inc : data.recursive_includes){
                if(this->headers[inc].fileWriteTime > cacheWriteTime){
                    cached = false;
                    break;
                }
            }
        }
        if(cached){
            if(!Silent) std::cout << "Reading Cached Shader " << cacheLoc << std::endl;

            std::ifstream cache(cacheLoc.c_str(), std::ios::binary);

            int length;
            GLenum binaryFormat;
            cache.read((char*)&length, 4);
            cache.read((char*)&binaryFormat, 4);
            std::vector<char> buff(length);
            cache.read(buff.data(), length);

            // get the blob
            glProgramBinary(data.programID, binaryFormat, buff.data(), length);

            // check link status
            GLint linkRes = 0;
            glGetProgramiv(data.programID, GL_LINK_STATUS, &linkRes);
            if (linkRes == GL_TRUE) {
                data.compiledSuccessfully = true;
                return data; // all good
            }
        }
    }

    bool success = true;
    for(auto& t : data.sources){
        success &= compileSource(t.second);
        if(!success)break;
    }

    if(success){
        for(const auto& t : data.sources){
            glAttachShader(data.programID, t.second.ID);
        }
        glLinkProgram(data.programID);

        GLint linkRes = 0;
        glGetProgramiv(data.programID, GL_LINK_STATUS, &linkRes);
        std::string program_path = data.sources.find(GL_COMPUTE_SHADER) != data.sources.end() ? data.sources.find(GL_COMPUTE_SHADER)->second.path : data.sources.find(GL_VERTEX_SHADER)->second.path;
        getLog(program_path);
        if (linkRes == GL_FALSE) {
            std::cout << "Error while linking shader " <<program_path << std::endl;
            success = false;
        }

        if(success){
            GLint validateStatus = 0;
            glValidateProgram(data.programID);
            glGetProgramiv(data.programID, GL_VALIDATE_STATUS, &validateStatus);
            getLog(program_path);
            if (validateStatus == GL_FALSE) {
                std::cout << "Error while validating shader " <<program_path << std::endl;
                success = false;
            }
        }

        for(const auto& t : data.sources){
            glDetachShader(data.programID, t.second.ID);
            glDeleteShader(t.second.ID);
        }
    }

    if(success){
        // write the binary blob
        int length;
        glGetProgramiv(data.programID, GL_PROGRAM_BINARY_LENGTH, &length);

        std::vector<char> buff(length);
        GLenum binaryFormat;
        glGetProgramBinary(data.programID, buff.size(), &length, &binaryFormat, buff.data());

        std::filesystem::create_directories(cacheLoc.parent_path());
        std::ofstream cache(cacheLoc.c_str(), std::ios::binary);
        cache.write((char*)&length, 4);
        cache.write((char*)&binaryFormat, 4);
        cache.write(buff.data(), length);

        // write the cache of all the shaders making up that program
        for(const auto& source : data.sources){
            writeCache(source.second);
        }
    }

    data.compiledSuccessfully = success;
    return data;
}

std::string GLShaderLoader::replaceLineNumberWithSourcePath(std::string &msg, const std::string& source_path) {
    auto words_begin =
            std::sregex_iterator(msg.begin(), msg.end(), replace_err_line_with_path);
    auto words_end = std::sregex_iterator();

    std::string result;
    result.reserve(msg.size() * 2);

    int last_pos = 0;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        result += msg.substr(last_pos, match.position(0) - last_pos);
        result += source_path;
        last_pos = match.position(0) + 1;
    }
    result += msg.substr(last_pos, msg.npos - last_pos);
    return result;
}

Shader GLShaderLoader::load(const char *computeFilePath) {
    return load({computeFilePath}, {GL_COMPUTE_SHADER});
}

Shader GLShaderLoader::load(const char *vertexFilePath, const char *fragmentFilePath) {
    return load({vertexFilePath, fragmentFilePath}, {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER});
}

Shader GLShaderLoader::load(const char *vertexFilePath, const char *geometryFilePath, const char *fragmentFilePath) {
    return load({vertexFilePath, geometryFilePath, fragmentFilePath},
                {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER});
}

Shader GLShaderLoader::load(const char *vertexFilePath, const char *tessellationControlFilePath,
                            const char *tessellationEvaluationFilePath, const char *geometryFilePath,
                            const char *fragmentFilePath) {
    return load({vertexFilePath, tessellationControlFilePath, tessellationEvaluationFilePath, geometryFilePath, fragmentFilePath},
                {GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER});
}

Shader GLShaderLoader::load(const std::vector<std::string> &localPaths, std::vector<GLenum> types) {
    Shader shader;

    if(!GLShaderLoader::instance){
        throw std::string("GLShaderLoader must be initialized before attempting to load a shader.");
    }

    GLShaderLoader& loader = *GLShaderLoader::instance;

    shader.data = loader.loadShaderData(localPaths, types, loader.loadingOrder);
    loader.loadingOrder++;

    if(!shader.data.compiledSuccessfully){
        std::cout <<"Shader compile error" <<std::endl;
        exit(-1);
    }else{
        for(auto& it : shader.data.sources){
            loader.setListener(it.second.path, &shader);
        }
        shader.deleter = GLShaderLoader::removeListener;
    }

    return shader;
}

void GLShaderLoader::checkForFileUpdates() {

    std::vector<std::string> updatedHeaders;

    for(int i=0; i<(int)headers_paths.size(); i++){
        ShaderHeader& header = headers[headers_paths[i]];
        uint64_t fileWriteTime = std::filesystem::last_write_time(headers_paths[i]).time_since_epoch().count();
        bool shouldReload = fileWriteTime > header.fileWriteTime;

        if(!shouldReload){
            for(const std::string& f : updatedHeaders){
                if(header.recursive_includes.contains(f)){
                    shouldReload = true;
                    break;
                }
            }
        }

        if(shouldReload){
            header = loadHeader(header.path); // reload the header
            updatedHeaders.push_back(headers_paths[i]);
        }

    }

    for(auto& it : shaders){
        ShaderProgram& data = it.second->data;
        bool needsReloading = false;
        for(auto& [source_type, source] : data.sources){
            uint64_t fileWriteTime = std::filesystem::last_write_time(source.path).time_since_epoch().count();
            if(fileWriteTime > source.fileWriteTime){
                needsReloading = true;
                break;
            }
        }

        for(const std::string& f : updatedHeaders){
            if(data.recursive_includes.contains(f)){
                needsReloading = true;
                break;
            }
        }

        if(needsReloading){
            std::vector<std::string> paths;
            std::vector<GLenum> types;
            for(auto& it2 : data.sources){
                paths.push_back(it2.second.localPath);
                types.push_back(it2.second.type);
            }
            ShaderProgram newData = loadShaderData(paths, types, data.loadingOrder);

            if(newData.compiledSuccessfully){
                data = newData;
                it.second->init_uniforms(it.second->uniforms_names);
            }
        }
    }

}

void GLShaderLoader::removeListener(Shader& shader){
    glUseProgram(0);
    glDeleteProgram(shader.data.programID);
    shader.data.programID = 0;
    for(auto& it : shader.data.sources){
        GLShaderLoader::instance->setListener(it.second.path, nullptr);
    }
}

