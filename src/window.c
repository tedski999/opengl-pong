#include "window.h"
#include "core.h"
#include "renderer.h"
#include "events.h"
#include "log.h"
#include "error.h"
#include <GLFW/glfw3.h>

static void pong_window_internal_errorCallback(int code, const char *description);
static void pong_window_internal_focusCallback(GLFWwindow *context, int is_focused);
static void pong_window_internal_closeCallback(GLFWwindow *context);

static GLFWwindow *window;

void pong_window_init() {
	PONG_LOG("Initializing GLFW window...", PONG_LOG_INFO);
	PONG_LOG("Using GLFW v%i.%i", PONG_LOG_INFO, GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR);
	glfwSetErrorCallback(pong_window_internal_errorCallback);
	if (!glfwInit())
		PONG_ERROR("Failed to initialize GLFW!");

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, PONG_OPENGL_VERSION_MAJOR_MIN);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, PONG_OPENGL_VERSION_MINOR_MIN);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	PONG_LOG("Opening window...", PONG_LOG_VERBOSE);
	window = glfwCreateWindow(PONG_WINDOW_WIDTH, PONG_WINDOW_HEIGHT, "Pong", NULL, NULL);
	if (!window)
		PONG_ERROR("Failed to create GLFW window!");

	PONG_LOG("Configuring window...", PONG_LOG_VERBOSE);
	glfwSetWindowCloseCallback(window, pong_window_internal_closeCallback);
	glfwSetWindowFocusCallback(window, pong_window_internal_focusCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	PONG_LOG("GLFW window initialized!", PONG_LOG_VERBOSE);

	pong_renderer_init();
}

void pong_window_update(void) {
	glfwPollEvents();
}

void pong_window_render(void) {
	glfwSwapBuffers(window);
	pong_renderer_clearScreen();
}

void pong_window_cleanup(void) {
	PONG_LOG("Cleaning up GLFW window...", PONG_LOG_INFO);
	if (window)
		glfwDestroyWindow(window);
	glfwTerminate();
	pong_renderer_cleanup();
}

static void pong_window_internal_errorCallback(int code, const char *description) {
	PONG_LOG("GLFW ERROR %i: %s", PONG_LOG_WARNING, code, description);
}

static void pong_window_internal_focusCallback(GLFWwindow *context, int is_focused) {
	PONG_LOG("GLFW window focus callback executed!", PONG_LOG_VERBOSE);
	pong_events_pushEvent(PONG_EVENT_FOCUS, is_focused);
}

static void pong_window_internal_closeCallback(GLFWwindow *context) {
	PONG_LOG("GLFW window close callback executed!", PONG_LOG_VERBOSE);
	pong_events_pushEvent(PONG_EVENT_QUIT);
}

