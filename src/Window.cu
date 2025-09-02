//
// Created by Briac on 18/06/2025.
//

#include "Window.cuh"

#include <iostream>
#include <cstdint>

#include <cuda_gl_interop.h>
#include <cuda_runtime_api.h>
#include "./RenderingBase/helper_cuda.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "RenderingBase/Camera.h"
#include "RenderingBase/GLShaderLoader.h"
#include "RenderingBase/GLIntrospection.h"
#include "RenderingBase/CudaIntrospection.cuh"

#include "PointCloudLoader.h"

#include <thread>
#include <chrono>

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
    fflush(stderr);
}

static void myGlDebugCallback(GLenum source,
                              GLenum type,
                              GLuint id,
                              GLenum severity,
                              GLsizei length,
                              const GLchar *message,
                              const void *userParam){

    if(severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM
    || severity == GL_DEBUG_SEVERITY_LOW){
        std::cout <<"GL_DEBUG: " <<message <<std::endl;
    }


}
static void glad_callback_custom(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLenum error_code;

    error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR) {
        std::string type("UNKNOWN");
        if (error_code == GL_INVALID_ENUM) {
            type = "GL_INVALID_ENUM";
        } else if (error_code == GL_INVALID_OPERATION) {
            type = "GL_INVALID_OPERATION";
        } else if (error_code == GL_INVALID_VALUE) {
            type = "GL_INVALID_VALUE";
        } else if (error_code == GL_INVALID_INDEX) {
            type = "GL_INVALID_INDEX";
        } else if (error_code == GL_INVALID_FRAMEBUFFER_OPERATION) {
            type = "GL_INVALID_FRAMEBUFFER_OPERATION";
        } else if (error_code == GL_OUT_OF_MEMORY) {
            type = "GL_OUT_OF_MEMORY";
        } else if(error_code == GL_CONTEXT_LOST){
            type = "GL_CONTEXT_LOST";
        }

        std::cout << "ERROR " << error_code << " in " << name << " (" << type
                  << ")" << std::endl;

        if (error_code == GL_OUT_OF_MEMORY) {
            throw std::string("OpenGL Fatal Error: Out of memory");
        }
    }
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
    glViewport(0, 0, width, height);
}

static double scroll;
static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
    scroll = yoffset;
}

Window::Window(const std::string &title, int samples) {
    if (!glfwInit())
        throw "Error while initializing GLFW";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_API, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
//    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

    glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, GLFW_LOSE_CONTEXT_ON_RESET);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
//    glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);

//    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
//    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);

    w = glfwCreateWindow(800, 600, title.c_str(), NULL, NULL);
    if (!w) {
        glfwTerminate();
        throw std::string("Error while creating the window");
    }

    glfwMakeContextCurrent(w);
    gladLoadGL(glfwGetProcAddress);
    gladInstallGLDebug();

    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << std::endl;

    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback(w, framebuffer_size_callback);
    glfwSetScrollCallback(w, scroll_callback);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    gladSetGLPostCallback(reinterpret_cast<GLADpostcallback>(glad_callback_custom));
    glDebugMessageCallback(myGlDebugCallback, nullptr);

//    guis = std::make_unique<GUIs>();
//    guis->init_IMGUI(w);
//    reloadFonts();
//
//    setTitle();
//    toogleFullscreen();
//    toogleVsync();
    auto ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(ctx);
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL(w, true);
}

Window::~Window() {

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void loadHeaders(){

    std::function<void(std::unordered_map<std::string, std::string>&)> replacements;
    std::unordered_map<std::string, std::string> m;

    // glsl syntax hacks
    m["___flat"] = "flat";
    m["___out"] = "out";
    m["___in"] = "in";
    m["___inout"] = "inout";
    m["___discard"] = "discard";
    m["//--"] = "";
    m["/\\*--"] = "";
    m["--\\*/"] = "";

    m["#include"] = "";
    m["\\bstatic\\b"] = "";
    m["\\binline\\b"] = "";
    m["__UNKOWN_SIZE"] = "";


    std::string regex_str = "";
    for(const auto& [s, d] : m){
        if(regex_str.size() > 0){
            regex_str += "|" + s;
        }else{
            regex_str += s;
        }
    }
    std::regex re = std::regex(regex_str);

    std::vector<std::string> headers;
    headers.push_back("resources/shaders/common/GLSLDefines.h");
    headers.push_back("resources/shaders/common/CommonTypes.h");
    headers.push_back("resources/shaders/common/Uniforms.h");
    headers.push_back("resources/shaders/common/Covariance.h");
    GLShaderLoader::instance->loadHeaders(headers, m, re);
}

void Window::mainloop(int argc, char **argv) {

    Camera camera;
    GLShaderLoader shaderLoader("resources/shaders", "SparseVoxelReconstruction");
    loadHeaders();

    unsigned int gl_device_count;
    int gl_device_id;
    checkCudaErrors(cudaGLGetDevices(&gl_device_count, &gl_device_id, 1, cudaGLDeviceListAll));
    int cuda_device_id = gl_device_id;
    checkCudaErrors(cudaSetDevice(cuda_device_id));

    cudaDeviceProp props;
    checkCudaErrors(cudaGetDeviceProperties(&props, gl_device_id));
    printf("GL   : %-24s (%2d SMs)\n", props.name, props.multiProcessorCount);
    checkCudaErrors(cudaGetDeviceProperties(&props, cuda_device_id));
    printf("CUDA : %-24s (%2d SMs)\n", props.name, props.multiProcessorCount);

//    CudaBufferSetupBoundsCheck();

    GaussianCloud cloud;
    cloud.initShaders();

    bool windowHovered = false;
    while (!glfwWindowShouldClose(this->w)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::BeginMainMenuBar();
        GLIntrospection::inspectObjects();
        CudaIntrospection::inspectBuffers();
        ImGui::EndMainMenuBar();

        ImGui::Begin("Window");
//        ImGui::ShowMetricsWindow();

        if(ImGui::Button("Reload Shaders")){
            shaderLoader.checkForFileUpdates();
        }
        if(ImGui::Button("Load ply")){
            PointCloudLoader::load(cloud, "bicycle.ply", true);
        }
        if(cloud.initialized){
            ImGui::Text("The point cloud contains %d 3D gaussians.", cloud.num_gaussians);
        }

        camera.updateView(w, windowHovered, (float)scroll);


        int width, height;
        glfwGetWindowSize(w, &width, &height);
        glfwGetFramebufferSize(w, &width, &height);

        glViewport(0, 0, width, height); // reset the viewport
//        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        // need to clear with alpha = 1 for front to back blending
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

        if(cloud.initialized){
            cloud.GUI(camera);
            cloud.render(camera);
        }

        windowHovered = ImGui::GetIO().WantCaptureMouse;

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        scroll *= 0.90f;

        glfwSwapBuffers(this->w);
        glfwPollEvents();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if(windowHovered){
            scroll = 0;
        }

    }
}