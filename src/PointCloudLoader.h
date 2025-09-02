//
// Created by Briac on 27/08/2025.
//

#ifndef HARDWARERASTERIZED3DGS_POINTCLOUDLOADER_H
#define HARDWARERASTERIZED3DGS_POINTCLOUDLOADER_H

#include <string>

#include "GaussianCloud.h"

class PointCloudLoader {
public:
    static void load(GaussianCloud& dst, const std::string& path, bool cudaGLInterop=true);
};


#endif //HARDWARERASTERIZED3DGS_POINTCLOUDLOADER_H
