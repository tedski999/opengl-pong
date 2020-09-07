#include "window.h"
#include "core.h"
#include "renderer.h"
#include "events.h"
#include "log.h"
#include <GLFW/glfw3.h>
#include <stdbool.h>

static void pong_window_internal_errorCallback(int code, const char *description);
static void pong_window_internal_focusCallback(GLFWwindow *context, int is_focused);
static void pong_window_internal_closeCallback(GLFWwindow *context);

static bool safe_to_clean = false;
static GLFWwindow *window;

int pong_window_init() {
	PONG_LOG("Initializing GLFW window...", PONG_LOG_INFO);
	PONG_LOG("Using GLFW v%i.%i", PONG_LOG_INFO, GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR);
	glfwSetErrorCallback(pong_window_internal_errorCallback);
	if (!glfwInit()) {
		PONG_LOG("Failed to initialize GLFW!", PONG_LOG_ERROR);
		return 1;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PONG_OPENGL_VERSION_MAJOR_MIN);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PONG_OPENGL_VERSION_MINOR_MIN);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	PONG_LOG("Opening window...", PONG_LOG_VERBOSE);
	window = glfwCreateWindow(PONG_WINDOW_WIDTH, PONG_WINDOW_HEIGHT, "Pong", NULL, NULL);
	if (!window) {
		PONG_LOG("Failed to create GLFW window!", PONG_LOG_ERROR);
		return 1;
	}
	safe_to_clean = true;

	PONG_LOG("Configuring window...", PONG_LOG_VERBOSE);
	glfwSetWindowCloseCallback(window, pong_window_internal_closeCallback);
	glfwSetWindowFocusCallback(window, pong_window_internal_focusCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	PONG_LOG("GLFW window initialized!", PONG_LOG_VERBOSE);

	if (pong_renderer_init())
		return 1;
	return 0;
}

void pong_window_update() {
	glfwPollEvents();
}

void pong_window_render() {
	glfwSwapBuffers(window);
	pong_renderer_clearScreen();
}

void pong_window_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up GLFW window...", PONG_LOG_INFO);
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	pong_renderer_cleanup();
}

void pong_window_internal_errorCallback(int code, const char *description) {
	PONG_LOG("GLFW ERROR %i: %s", PONG_LOG_WARNING, code, description);
}

void pong_window_internal_focusCallback(GLFWwindow *context, int is_focused) {
	PONG_LOG("GLFW window focus callback executed!", PONG_LOG_VERBOSE);
	pong_events_pushEvent(PONG_EVENT_FOCUS, is_focused);
}

void pong_window_internal_closeCallback(GLFWwindow *context) {
	PONG_LOG("GLFW window close callback executed!", PONG_LOG_VERBOSE);
	pong_events_pushEvent(PONG_EVENT_QUIT);
}

