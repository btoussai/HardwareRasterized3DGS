#ifndef SRC_RENDERINGBASE_GLINTROSPECTION_H
#define SRC_RENDERINGBASE_GLINTROSPECTION_H

#include "../glad/gl.h"

#include <unordered_set>
#include <string>

class GLIntrospection {
public:
	static void addTexture(GLuint ID);
	static void removeTexture(GLuint ID);

	static void addBuffer(GLuint ID);
	static void removeBuffer(GLuint ID);

    static void addVAO(GLuint ID);
    static void removeVAO(GLuint ID);

	static void inspectObjects();

    static std::string prettyPrintMem(size_t bytes);
    static int getBytesPerTexel(GLenum internalFormat);
    static std::string getFormatName(GLenum internalFormat);

private:

	static std::unordered_set<GLuint> textures;
	static std::unordered_set<GLuint> buffers;
    static std::unordered_set<GLuint> vaos;

};

#endif /* SRC_RENDERINGBASE_GLINTROSPECTION_H */
