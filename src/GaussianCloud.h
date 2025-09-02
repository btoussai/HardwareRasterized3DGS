//
// Created by Briac on 27/08/2025.
//

#ifndef HARDWARERASTERIZED3DGS_GAUSSIANCLOUD_H
#define HARDWARERASTERIZED3DGS_GAUSSIANCLOUD_H

#include "RenderingBase/GLBuffer.h"
#include "RenderingBase/Shader.h"
#include "RenderingBase/GLShaderLoader.h"
#include "RenderingBase/Camera.h"
#include "RenderingBase/VAO.h"
#include "RenderingBase/GLTimer.h"
#include "Sort.cuh"

class GaussianCloud {
public:
    bool initialized = false;
    int num_gaussians;

    std::vector<glm::vec4> positions_cpu;
    std::vector<glm::vec4> scales_cpu;
    std::vector<glm::vec4> rotations_cpu;
    std::vector<float> opacities_cpu;

    // values for all the gaussians
    GLBuffer positions; // x, y, z, padding
    GLBuffer scales;    // sx, sy, sz, padding
    GLBuffer rotations; // rx, ry, rz, rw
    GLBuffer opacities; // alpha
    GLBuffer sh_coeffs[3]; // 3 color channels, 16 coeffs each

    // values only for the gaussians that are visible, packed tightly without gaps
    GLBuffer conic_opacity;
    GLBuffer bounding_boxes; // 2D bounding boxes of the visible gaussians
    GLBuffer eigen_vecs;
    GLBuffer predicted_colors; // view-dependent colors
    GLBuffer gaussians_indices; // indices of the visible gaussians
    GLBuffer gaussians_depths;
    GLBuffer visible_gaussians_counter; // number of visible gaussians

    GLBuffer sorted_depths;
    GLBuffer sorted_gaussian_indices;

    void initShaders();
    void GUI(Camera& camera);
    void render(Camera& camera);

private:
    Shader pointShader = GLShaderLoader::load("point.vs", "point.fs");
    Shader quadShader = GLShaderLoader::load("quad.vs", "quad.fs");
    Shader testVisibilityShader = GLShaderLoader::load("testVisibility.cp");
    Shader computeBoundingBoxesShader = GLShaderLoader::load("computeBoundingBoxes.cp");
    Shader predictColorsShader = GLShaderLoader::load("predict_colors.cp");
    Shader predictColorsForAllShader = GLShaderLoader::load("predict_colors_for_all.cp");

    void prepareRender(Camera& camera);

    GLBuffer uniforms;
    VAO quad;

    Sort sort;

    int num_visible_gaussians = 0;
    bool renderAsPoints = true;
    bool renderAsQuads = false;
    float scale_modifier = 1.0f;
    bool antialiasing = false;
    float min_opacity = 0.02f;
    bool front_to_back = true;
    int selected_gaussian = -1;

    enum OPERATIONS{
        PREDICT_COLORS_ALL,
        DRAW_AS_POINTS,
        TEST_VISIBILITY,
        SORT,
        COMPUTE_BOUNDING_BOXES,
        PREDICT_COLORS_VISIBLE,
        DRAW_AS_QUADS,
        NUM_OPS
    };

    QueryBuffer timers[OPERATIONS::NUM_OPS];

    GLBuffer counter;
};


#endif //HARDWARERASTERIZED3DGS_GAUSSIANCLOUD_H
