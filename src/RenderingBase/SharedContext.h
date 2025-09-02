#ifndef SEQUENCE_SHAREDCONTEXT_H_
#define SEQUENCE_SHAREDCONTEXT_H_

#include "../glad/gl.h"
#include <GLFW/glfw3.h>

#include <thread>
#include <functional>
#include "ThreadSafeQueue.h"

#ifdef __linux__
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#elif _WIN32
typedef void *EGLDisplay;
typedef void *EGLContext;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef unsigned int EGLBoolean;
typedef khronos_int32_t EGLint;
#endif


struct EGL_Data{
	EGLDisplay eglDisplay;
	EGLContext eglContext;
	EGLConfig  eglConfig;
	EGLContext (*eglCreateContext) (EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
	EGLBoolean (*eglMakeCurrent) (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
};

/**
 * A background thread holding a second OpenGL context
 */
class SharedContext {
public:
	SharedContext(GLFWwindow* mainWindow, EGL_Data eglData);
	virtual ~SharedContext();

	void scheduleTask(std::function<void()>&& task);
	void scheduleTaskOnFinish(std::function<void()>&& task);
	void waitForAll();

private:
	GLFWwindow* w;
	EGL_Data eglData;
	EGLContext eglContext;

	std::atomic_bool should_exit = false;
	std::unique_ptr<std::thread> thread;
	ThreadSafeQueue<std::function<void()>> tasks;
	ThreadSafeQueue<std::function<void()>> onFinishTasks; // tasks to be run by the main thread in the waitForAll method
	std::atomic_int pending_tasks = 0;

	std::mutex m;
	std::condition_variable condition;

	void thread_loop();
};

#endif /* SEQUENCE_SHAREDCONTEXT_H_ */
