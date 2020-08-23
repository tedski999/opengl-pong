#include "window.h"
#include "renderer.h"
#include "events.h"
#include "log.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

static bool safe_to_clean = false;

static void pong_window_errorCallback(int code, const char *description);
static void pong_window_closeCallback(GLFWwindow *context);

static GLFWwindow *window;

int pong_window_init() {
	PONG_LOG("Initializing GLFW window...", PONG_LOG_VERBOSE);
	glfwSetErrorCallback(pong_window_errorCallback);
	if (!glfwInit()) {
		PONG_LOG("Failed to initialize GLFW!", PONG_LOG_ERROR);
		return 1;
	}
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong", NULL, NULL);
	if (!window) {
		PONG_LOG("Failed to create GLFW window!", PONG_LOG_ERROR);
		return 1;
	}
	safe_to_clean = true;
	glfwSetWindowCloseCallback(window, pong_window_closeCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	if (pong_renderer_init())
		return 1;
	return 0;
}

void pong_window_update() {
	glfwPollEvents();
}

void pong_window_render() {
	glfwSwapBuffers(window);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void pong_window_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up GLFW window...", PONG_LOG_VERBOSE);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	pong_renderer_cleanup();
}

void pong_window_errorCallback(int code, const char *description) {
	PONG_LOG("GLFW ERROR %i: %s", PONG_LOG_WARNING, code, description);
}

void pong_window_closeCallback(GLFWwindow *context) {
	PONG_LOG("GLFW window close callback executed!", PONG_LOG_VERBOSE);
	pong_events_pushEvent(PONG_EVENT_QUIT);
}

