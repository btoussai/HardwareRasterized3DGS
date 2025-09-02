//
// Created by Briac on 18/06/2025.
//

#include "Camera.h"

#include "../imgui/imgui.h"

#include "../glm/ext/matrix_transform.hpp"
#include "../glm/gtc/matrix_inverse.hpp"
#include "../glm/ext/matrix_clip_space.hpp"

using namespace glm;

const float PI = (float)std::numbers::pi;

Camera::Camera() {
    camDir = - vec3(sin(theta) * cos(phi), sin(phi),
                         cos(theta) * cos(phi));
    lookPos = vec3(0);
    camPos = -camDir * dist2lookPos + lookPos;
}

Camera::~Camera() {

}

void Camera::updateView(GLFWwindow *window, bool windowHovered, float scroll) {
    static double xpos = 0, ypos = 0;
    double new_xpos, new_ypos;
    glfwGetCursorPos(window, &new_xpos, &new_ypos);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    ctrlKey = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    altKey =  glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    shiftKey =  glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

    framebufferSize = ivec2(width, height);
    mouseFramebufferCoords = vec2(new_xpos, new_ypos);
    mouseNormalizedCoords = vec2(2.0f * xpos / width - 1.0f,
                        1.0f - 2.0f * ypos / height);

    float dx = new_xpos - xpos;
    float dy = new_ypos - ypos;
    xpos = new_xpos;
    ypos = new_ypos;

    bool lmbPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rmbPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool cmbPressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    if (movementsEnabled && lmbPressed && !windowHovered) {
        theta -= dx * 0.005f;
        phi += dy * 0.005f;
        phi = clamp(phi, -PI * 0.49f, +PI * 0.49f);
    }

    ImGui::Spacing();

    ImGui::Checkbox("Free camera", &this->freeCam);
    if(this->freeCam){
        ImGui::SliderFloat("Cam speed", &camSpeed, 0, 0.5f * 1.0f / 60.0f, "%.5f");
    }
    ImGui::Spacing();


    vec3 up = vec3(0, 1, 0);
    if (!freeCam) {
        if (!windowHovered) {
            if(movementsEnabled){
                dist2lookPos *= (1.0f - scroll * 0.02f);
            }
            if (dist2lookPos > farPlane / 2.0) {
                dist2lookPos = farPlane / 2.0;
            }
        }
        scroll = 0;

        camDir = -vec3(sin(theta) * cos(phi), sin(phi),
                            cos(theta) * cos(phi));


        camPos = -camDir * dist2lookPos + lookPos;
        viewMat = glm::lookAt(camPos, lookPos, up);

        if(movementsEnabled && ((ctrlKey && lmbPressed) || rmbPressed)){
            // panning
            lookPos += camRight * -dx * dist2lookPos / (float)width;
            lookPos += camUp * dy * dist2lookPos / (float)height;
        }
    } else {
        // free cam
        if(movementsEnabled && !windowHovered){
            bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
            bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

            bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
            bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

            bool up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
            bool down = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

            if (forward) {
                camPos += vec3(sin(theta), 0, cos(theta)) * -camSpeed;
            } else if (backward) {
                camPos += vec3(sin(theta), 0, cos(theta)) * camSpeed;
            }
            if (left) {
                camPos += vec3(cos(theta), 0, -sin(theta)) * -camSpeed;
            } else if (right) {
                camPos += vec3(cos(theta), 0, -sin(theta)) * camSpeed;
            }
            if (up) {
                camPos += vec3(0, 1, 0) * camSpeed;
            } else if (down) {
                camPos += vec3(0, 1, 0) * -camSpeed;
            }
        }

        camDir = -vec3(sin(theta) * cos(phi), sin(phi),
                            cos(theta) * cos(phi));
        viewMat = lookAt(camPos, camPos + camDir, up);
    }

    camDir /= glm::length(camDir);
    camRight = glm::cross(camDir, up);
    camRight /= glm::length(camRight);
    camUp = glm::cross(camRight, camDir);
    camUp /= glm::length(camUp);

    invViewMat = glm::inverse(viewMat);

    if (width != 0 && height != 0) {
        projMat = glm::perspective(fovY,
                                   (float) width / (float) height, nearPlane, farPlane);
        invProj = glm::inverse(projMat);
    }

    projViewMat = projMat * viewMat;
    invProjViewMat = glm::inverse(projViewMat);


}

glm::mat4 Camera::getProjectionViewMatrix() const {
    return projViewMat;
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMat;
}

glm::mat4 Camera::getProjectionMatrix() const {
    return projMat;
}

glm::vec2 Camera::getMouseFramebufferCoords() const {
    return mouseFramebufferCoords;
}

glm::ivec2 Camera::getFramebufferSize() const {
    return framebufferSize;
}

float Camera::getNearPlane() const {
    return nearPlane;
}

float& Camera::getFarPlane() {
    return farPlane;
}

glm::vec3 Camera::getPosition() const {
    return camPos;
}

float Camera::getFovX() const {
    // width / height
    float aspect = (float) framebufferSize.x / (float) framebufferSize.y;
    float fovX = 2.0f * atan(tan(fovY * 0.5) * aspect);
    return fovX;
}

float Camera::getFovY() const {
    return fovY;
}
