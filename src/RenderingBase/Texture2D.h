//
// Created by Briac on 19/06/2025.
//

#ifndef SPARSEVOXRECON_TEXTURE2D_H
#define SPARSEVOXRECON_TEXTURE2D_H

#include "../glad/gl.h"
#include <cuda_gl_interop.h>
#include <cstdint>
#include <string>
#include <functional>
#include <memory>

class Texture2D {
public:
    struct TextureData{
        std::string path;
        int width;
        int height;
        GLenum internalFormat;
        GLenum format;
        GLenum type;
        void* ptr; // Can be actual data or clear data or nullptr
        std::function<void(void*)> deleter; // to be called on ptr

        TextureData() = default;
        TextureData(const std::string& path, int width, int height, GLenum internalFormat, GLenum format, GLenum type,
                    unsigned char *ptr, const std::function<void(void *)>& deleter);
        ~TextureData(){
            deleter(ptr);
        }

        TextureData(const TextureData&) = delete;
        TextureData& operator=(const TextureData&) = delete;

        TextureData(TextureData&&) noexcept ;
        TextureData& operator=(TextureData&&);
    };

    static TextureData readTextureFromDisk(const std::string& path, bool Hflip, int req_channels);

    explicit Texture2D(const TextureData& data, bool clear=false, bool useCudaGLinterop=false, bool makeResident=true, int mipmaps=1);
    virtual ~Texture2D();

    Texture2D(Texture2D&&) = delete;
    Texture2D& operator=(Texture2D&&) = delete;

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    void setFilter(GLenum minFilter, GLenum magFilter);
    void setWrapMode(GLenum wrapMode);

    GLuint getID() const;
    cudaSurfaceObject_t getCudaSurfObj() const;
    uint64_t getImageHandle() const;
    uint64_t getTextureHandle() const;
    const TextureData& getTextureData() const;

    void makeHandlesResident(bool resident);
    void makeTextureHandleResident(bool resident);
    void makeImageHandleResident(bool resident, GLenum access);
    size_t memUsage() const;

    int getWidth() const;
    int getHeight() const;

private:
    GLuint ID = 0;
    GLenum wrapMode = GL_CLAMP_TO_EDGE;
    GLenum minFilter = GL_NEAREST;
    GLenum magFilter = GL_NEAREST;
    uint64_t tex_handle = 0;
    uint64_t image_handle = 0;

    bool useCudaGLinterop = false;
    cudaGraphicsResource_t cuda_handle = nullptr;
    cudaSurfaceObject_t surfObj = 0;

    TextureData data;

    void makeBindless();
};


#endif //SPARSEVOXRECON_TEXTURE2D_H
