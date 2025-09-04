//
// Created by Briac on 28/08/2025.
//

#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include "GLSLDefines.h"

struct Uniforms{
    mat4 viewMat;
    mat4 projMat;

    vec4 camera_pos;

    int num_gaussians;
    float near_plane;
    float far_plane;
    float scale_modifier;

    int selected_gaussian;
    float min_opacity;
    float width;
    float height;

    float focal_x;
    float focal_y;
    int antialiasing;
    int front_to_back;

    vec4* restrict positions;
    vec4* restrict rotations;
    vec4* restrict scales;
    float* restrict opacities;
    float* restrict sh_coeffs_red;
    float* restrict sh_coeffs_green;
    float* restrict sh_coeffs_blue;

    float* restrict dLoss_dsh_coeffs_red;
    float* restrict dLoss_dsh_coeffs_green;
    float* restrict dLoss_dsh_coeffs_blue;

    int* restrict visible_gaussians_counter;
    float* restrict gaussians_depth;
    int* restrict gaussians_indices;
    float* restrict sorted_depths;
    int* restrict sorted_gaussian_indices;

    // compacted buffers, filled after sorting by depth
    vec4* restrict bounding_boxes; // vec4(center, oriented_bounding_box), in pixels
    vec4* restrict conic_opacity; // vec4(conic, opacity), with conic in pixels
    vec2* restrict eigen_vecs; // principal direction of the 2D ellipsoid corresponding to the largest eigen value
    vec4* restrict predicted_colors;

    f16vec4* restrict dLoss_dconic_opacity;
    f16vec4* restrict dLoss_dpredicted_colors;

    uint64_t ground_truth_image; // handle of the ground truth picture
    uint64_t accumulated_image_fwd;  // handle of the image used for alpha blending in the forward pass
};

#endif //COMMONTYPES_H
