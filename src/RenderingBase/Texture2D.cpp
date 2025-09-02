//
// Created by Briac on 19/06/2025.
//

#include "Texture2D.h"

#include "GLIntrospection.h"
#include "CudaGLInterop.h"
#include "helper_cuda.h"
#include "../stb/stb_image.h"
#include <iostream>
#include <sstream>
#include <functional>

GLuint Texture2D::getID() const {
    return ID;
}

cudaSurfaceObject_t Texture2D::getCudaSurfObj() const {
    return surfObj;
}

uint64_t Texture2D::getImageHandle() const {
    return image_handle;
}

uint64_t Texture2D::getTextureHandle() const {
    return tex_handle;
}

const Texture2D::TextureData &Texture2D::getTextureData() const {
    return data;
}

void Texture2D::makeBindless() {
    tex_handle = glGetTextureHandleARB(ID);
    image_handle = glGetImageHandleARB(ID, 0, false, 0, data.internalFormat);
}

void Texture2D::makeHandlesResident(bool resident) {
    makeTextureHandleResident(resident);
    makeImageHandleResident(resident, GL_READ_WRITE);
}

void Texture2D::makeTextureHandleResident(bool resident) {
    if (resident) {
        glMakeTextureHandleResidentARB(tex_handle);
    } else {
        glMakeTextureHandleNonResidentARB(tex_handle);
    }
}

void Texture2D::makeImageHandleResident(bool resident, GLenum access) {
    if (resident) {
        glMakeImageHandleResidentARB(image_handle, access);
    } else {
        glMakeImageHandleNonResidentARB(image_handle);
    }
}

size_t Texture2D::memUsage() const {
    return size_t(data.width) * size_t(data.height) * GLIntrospection::getBytesPerTexel(data.internalFormat);
}

void Texture2D::setWrapMode(GLenum wrapMode) {
    glTextureParameteri(ID, GL_TEXTURE_WRAP_S, wrapMode);
    glTextureParameteri(ID, GL_TEXTURE_WRAP_T, wrapMode);
    this->wrapMode = wrapMode;
}

void Texture2D::setFilter(GLenum minFilter, GLenum magFilter) {
    glTextureParameteri(ID, GL_TEXTURE_MIN_FILTER, minFilter);
    glTextureParameteri(ID, GL_TEXTURE_MAG_FILTER, magFilter);
    this->minFilter = minFilter;
    this->magFilter = magFilter;
}

Texture2D::~Texture2D() {
    GLIntrospection::removeTexture(ID);
    if (this->useCudaGLinterop) {
        checkCudaErrors(cudaDestroySurfaceObject(surfObj));
        CudaGLInterop::unregisterImage(&cuda_handle);
    }
    glDeleteTextures(1, &ID);
}

Texture2D::Texture2D(const Texture2D::TextureData &data, bool clear, bool useCudaGLinterop, bool makeResident,
                     int mipmaps) :
        data(data.path, data.width, data.height, data.internalFormat, data.format, data.type, nullptr, [](void*){})
{
    glCreateTextures(GL_TEXTURE_2D, 1, &ID);
    GLIntrospection::addTexture(ID);
    glTextureStorage2D(ID, 1, data.internalFormat, data.width, data.height);
    if (clear) {
        glClearTexImage(ID, 0, data.format, data.type, data.ptr);
    } else if (data.ptr) {
        glTextureSubImage2D(ID, 0, 0, 0, data.width, data.height, data.format, data.type, data.ptr);
    }

    setFilter(GL_NEAREST, GL_NEAREST);
    setWrapMode(GL_CLAMP_TO_EDGE);

    if (mipmaps > 1) {
        glGenerateTextureMipmap(ID);
    }

    if (useCudaGLinterop) {
        cuda_handle = CudaGLInterop::registerImage(ID, GL_TEXTURE_2D);
        checkCudaErrors(cudaGraphicsMapResources(1, &cuda_handle));
        cudaResourceDesc resDesc;
        memset(&resDesc, 0, sizeof(resDesc));
        resDesc.resType = cudaResourceTypeArray;
        cudaArray_t array = nullptr;
        checkCudaErrors(cudaGraphicsSubResourceGetMappedArray(&array, cuda_handle, 0, 0));
        resDesc.res.array.array = array;
        checkCudaErrors(cudaCreateSurfaceObject(&surfObj, &resDesc));
        checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_handle));
    } else {
        cuda_handle = nullptr;
    }

    makeBindless();
    if (makeResident) {
        makeHandlesResident(true);
    }

}

Texture2D::TextureData Texture2D::readTextureFromDisk(const std::string &path, bool Hflip, int req_channels) {

    stbi_set_flip_vertically_on_load(false);
    int width = 0, height = 0, nrChannels = 0;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, req_channels);

    if (!data) {
        std::stringstream ss;
        ss << "Couldn't read file " << path;
        throw ss.str();
    }
    //	std::cout << "Read texture data: " << path << " " << width << "x" << height
    //				<< " channels: " << nrChannels << std::endl;
    nrChannels = req_channels;

    if (Hflip) {
        for (int j = 0; j < height / 2; j++) {
            for (int i = 0; i < width; i++) {
                for (int c = 0; c < nrChannels; c++) {
                    std::swap(data[(j * width + i) * nrChannels + c],
                              data[((height - 1 - j) * width + i) * nrChannels + c]);
                }
            }
        }
    }

    GLenum internalFormat, format, type;
    if (nrChannels == 4) {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
    } else if (nrChannels == 2) {
        internalFormat = GL_RG8;
        format = GL_RG;
        type = GL_UNSIGNED_BYTE;
    } else if (nrChannels == 1) {
        internalFormat = GL_R8;
        format = GL_RED;
        type = GL_UNSIGNED_BYTE;
    } else {
        throw std::string("Unsupported number of channels");
    }

    std::function<void(void *)> del = [](void *ptr) {
        if (ptr) {
            stbi_image_free(ptr);
        }
    };
    return TextureData(path, width, height, internalFormat, format, type, data, del);
}

int Texture2D::getWidth() const {
    return data.width;
}

int Texture2D::getHeight() const {
    return data.height;
}

Texture2D::TextureData::TextureData(Texture2D::TextureData &&other) noexcept {
    *this = std::move(other);
}

Texture2D::TextureData &Texture2D::TextureData::operator=(Texture2D::TextureData &&other) {
    if (this == &other) return *this;

    deleter(ptr);

    path = other.path;
    width = other.width;
    height = other.height;
    internalFormat = other.internalFormat;
    format = other.format;
    type = other.type;
    ptr = other.ptr;
    deleter = other.deleter;

    other.ptr = nullptr;

    return *this;
}

Texture2D::TextureData::TextureData(const std::string &path, int width, int height, GLenum internalFormat,
                                    GLenum format,
                                    GLenum type, unsigned char *ptr, const std::function<void(void *)> &deleter) :
        path(path), width(width), height(height),
        internalFormat(internalFormat), format(format), type(type),
        ptr(ptr), deleter(deleter) {

}

