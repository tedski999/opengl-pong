#include "window.h"
#include "renderer.h"
#include "pong.h"
#include <GLFW/glfw3.h>

static GLFWwindow *window;

void window_close_callback(GLFWwindow *context) {
	pong_stop(); // TODO: replace with an event
}

int pong_window_init() {
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(640, 480, "Pong", NULL, NULL);
	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	pong_renderer_init();
	return 0;
}

void pong_window_update() {
	glfwPollEvents();
}

void pong_window_render() {
	glfwSwapBuffers(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // FIXME: remove after adding modern opengl
}

void pong_window_cleanup() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

