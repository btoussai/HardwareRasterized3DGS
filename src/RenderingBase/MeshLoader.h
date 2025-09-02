//
// Created by Briac on 19/06/2025.
//

#ifndef SPARSEVOXRECON_MESHLOADER_H
#define SPARSEVOXRECON_MESHLOADER_H

#include "VAO.h"

class MeshLoader {
public:
    static VAO loadMesh(const std::string& path, bool cudaGLInterop=false);

};


#endif //SPARSEVOXRECON_MESHLOADER_H
