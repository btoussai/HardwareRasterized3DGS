//
// Created by Briac on 28/08/2025.
//

#ifndef HARDWARERASTERIZED3DGS_SORT_CUH
#define HARDWARERASTERIZED3DGS_SORT_CUH

#include "RenderingBase/GLBuffer.h"

class Sort {
public:
    void sort(GLBuffer& depths, GLBuffer& sorted_depths, GLBuffer& indices, GLBuffer& sorted_indices, int count);
};


#endif //HARDWARERASTERIZED3DGS_SORT_CUH
