//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_CAMERA_H
#define SPARSEVOXRECON_CAMERA_H

#include "../glad/gl.h"
#include "GLFW/glfw3.h"

#include "../glm/vec2.hpp"
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"

#include <numbers>

class Camera {
public:
    Camera();
    virtual ~Camera();

    void updateView(GLFWwindow *window, bool windowHovered, float scroll);

    glm::vec3 getPosition() const;
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getProjectionViewMatrix() const;

    glm::vec2 getMouseFramebufferCoords() const;
    glm::ivec2 getFramebufferSize() const;

    float getNearPlane() const;
    float& getFarPlane();

    float getFovX() const;
    float getFovY() const;
private:
    float dist2lookPos = 3;
    float theta = 0;
    float phi = 0;

    float nearPlane = 0.001f;
    float farPlane = 100.0f;

    float fovY = (float) std::numbers::pi / 4.0f;
    float camSpeed = 0.25f / 60.0f;

    glm::mat4 projMat;
    glm::mat4 viewMat;

    glm::mat4 invProj;
    glm::mat4 invViewMat;

    glm::mat4 projViewMat;
    glm::mat4 invProjViewMat;

    glm::vec3 camPos;
    glm::vec3 lookPos;
    glm::vec3 camDir;
    glm::vec3 camRight;
    glm::vec3 camUp;

    bool freeCam = false;
    bool movementsEnabled = true; // set to false in 2D mode

    bool ctrlKey;
    bool altKey;
    bool shiftKey;

    glm::ivec2 framebufferSize;
    glm::vec2 mouseFramebufferCoords;
    glm::vec2 mouseNormalizedCoords;
};


#endif //SPARSEVOXRECON_CAMERA_H
