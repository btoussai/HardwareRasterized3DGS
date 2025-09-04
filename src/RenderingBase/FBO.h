//
// Created by Briac on 04/09/2025.
//

#ifndef HARDWARERASTERIZED3DGS_FBO_H
#define HARDWARERASTERIZED3DGS_FBO_H

#include "../glad/gl.h"
#include "Texture2D.h"
#include <unordered_map>
#include <memory>

class FBO {
public:
    FBO();
    ~FBO();
    void init(int width, int height);
    void reset();
    void makeEmpty();
    void createAttachment(GLenum attachment, GLenum internalFormat, GLenum format, GLenum type);
    bool checkComplete();
    void drawBuffersAllAttachments();
    void bind();
    void unbind();
    void blit(GLuint dstID, GLbitfield mask);

    int getWidth() const{return width;}
    int getHeight() const{return height;}
    GLuint getID() const{return ID;}
    std::unique_ptr<Texture2D>& getAttachment(GLenum attachment){
        return attachments[attachment];
    }

private:
    GLuint ID{};
    int width{};
    int height{};
    std::unordered_map<GLenum, std::unique_ptr<Texture2D>> attachments;
};


#endif //HARDWARERASTERIZED3DGS_FBO_H
