//
// Created by Briac on 04/09/2025.
//

#include "FBO.h"

#include "GLIntrospection.h"
#include "Texture2D.h"
#include <iostream>

FBO::FBO() {
}

FBO::~FBO() {
    reset();
}

void FBO::init(int width, int height) {
    reset();
    glCreateFramebuffers(1, &ID);
    this->width = width;
    this->height = height;
}

void FBO::reset() {
    if(ID == 0) return;
    glDeleteFramebuffers(1, &ID);
    ID = 0;
    width = height = 0;
    attachments.clear();
}

void FBO::createAttachment(GLenum attachment, GLenum internalFormat, GLenum format, GLenum type) {
    if(attachments.contains(attachment)){
        throw std::string("Error, FBO attachment already specified");
    }

    Texture2D::TextureData data = Texture2D::TextureData("", width, height, internalFormat, format, type, nullptr, [](void*){});
    auto& tex = attachments[attachment] = std::make_unique<Texture2D>(data, true, true, true, 1);
    glNamedFramebufferTexture(ID, attachment, tex->getID(), 0);

}

bool FBO::checkComplete() {
    GLenum res = glCheckNamedFramebufferStatus(ID, GL_DRAW_FRAMEBUFFER);
    if (res == GL_FRAMEBUFFER_COMPLETE) {
        return true;
    } else {
        std::cout <<"Framebuffer is incomplete:" <<std::endl;
        if (res == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
        }else if (res == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
        } else if (res == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
        } else if (res == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
        } else if (res == GL_FRAMEBUFFER_UNSUPPORTED) {
            std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
        } else if (res == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
        } else if (res == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
            std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
        } else if (res == 0) {
            std::cout << "UNKNOWN ERROR" << std::endl;
        }
    }

    return false;
}

void FBO::drawBuffersAllAttachments() {
    std::vector<GLenum> names;
    names.reserve(attachments.size());
    for(auto& [attachement, tex] : attachments){
        names.push_back(attachement);
    }
    glNamedFramebufferDrawBuffers(ID, (int)names.size(), names.data());
}

void FBO::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}

void FBO::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FBO::blit(GLuint dstID, GLbitfield mask) {
    glBlitNamedFramebuffer(ID, dstID,
            0, 0, width, height,
            0, 0, width, height,
            mask, GL_NEAREST);
}

void FBO::makeEmpty() {
    glNamedFramebufferParameteri(ID, GL_FRAMEBUFFER_DEFAULT_WIDTH, (int)width);
    glNamedFramebufferParameteri(ID, GL_FRAMEBUFFER_DEFAULT_HEIGHT, (int)height);
}
