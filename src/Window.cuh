//
// Created by Briac on 18/06/2025.
//

#ifndef SPARSEVOXRECON_WINDOW_CUH
#define SPARSEVOXRECON_WINDOW_CUH

#include <string>

#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include <memory>

class Window {
public:
    Window(const std::string& title, int samples);
    ~Window();

    void mainloop(int argc, char* argv[]);
private:
    GLFWwindow* w;
};


#endif //SPARSEVOXRECON_WINDOW_CUH
