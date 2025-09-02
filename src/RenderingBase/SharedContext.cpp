
#include "SharedContext.h"

#include <string>
#include <iostream>
#include <mutex>
#include <sstream>

SharedContext::SharedContext(GLFWwindow *mainWindow, EGL_Data eglData) {
	this->eglData = eglData;

	if(mainWindow){
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		w = glfwCreateWindow(640, 480, "SharedContext", NULL, mainWindow);
		if (!w) {
			glfwTerminate();
			throw std::string("Error while creating the shared context window");
		}
	}else{
		eglContext = eglData.eglCreateContext(eglData.eglDisplay, eglData.eglConfig, eglData.eglContext,
			NULL);
	}

	thread = std::make_unique<std::thread>([this](){
		thread_loop();
	});
}

SharedContext::~SharedContext() {
	should_exit = true;
	thread->join();

	std::cout << "Shared context deleted." << std::endl;
}

void SharedContext::scheduleTask(std::function<void()> &&task) {
	pending_tasks++;
	tasks.push(std::move(task));
}
void SharedContext::scheduleTaskOnFinish(std::function<void()>&& task){
	onFinishTasks.push(std::move(task));
}

void SharedContext::thread_loop() {

	if(w){
		glfwMakeContextCurrent(w);
	}else{
		eglData.eglMakeCurrent(eglData.eglDisplay, nullptr, nullptr, eglContext);
	}

	std::chrono::milliseconds timeout(100);
	while (!should_exit) {
		std::function<void()> task;
		bool success = false;
		int remaining = (int)tasks.get(task, timeout, success);

		if (should_exit) {
			break;
		}

		if (success) {
			try {
				task();
				pending_tasks--;
			} catch (std::string &s) {
				std::stringstream ss;
				ss << "An exception occurred in the shared context thread." << std::endl;
				ss << s << std::endl;
				std::cout << ss.str() << std::endl;
				exit(-1);
			}
		}

		if(remaining == 0){
			condition.notify_all();
		}
	}
}

void SharedContext::waitForAll() {
	std::unique_lock < std::mutex > lock(m);

	std::chrono::milliseconds timeout(100);
	bool success = false;
	while(!success){
		success = condition.wait_for(lock, timeout, [this]() {
			return tasks.size() == 0 && pending_tasks == 0;
		});
	}

    success = false;
    int remaining = 0;
    do{
        std::function<void()> task;
		remaining = (int)onFinishTasks.get(task, timeout, success);
		if (success) {
			task();
		}
	}while(remaining > 0);

}
