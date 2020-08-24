// TODO: modern opengl

#include "renderer.h"
#include "resources.h"
#include "log.h"
#include <GL/gl.h>
#include <stdbool.h>

static bool safe_to_clean = false;

int pong_renderer_init() {
	PONG_LOG("Initializing renderer...", PONG_LOG_VERBOSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PONG_LOG("Vendor: %s", PONG_LOG_INFO, glGetString(GL_VENDOR));
	PONG_LOG("Renderer: %s", PONG_LOG_INFO, glGetString(GL_RENDERER));
	safe_to_clean = true;

	//temp - testing archived file inflation
	PONG_LOG("Loading shaders...", PONG_LOG_VERBOSE);
	pong_resources_load("res/shaders/basic.vert", "basicVertShader");
	const char *vert = pong_resources_get("basicVertShader");
	pong_resources_load("res/shaders/basic.frag", "basicFragShader");
	const char *frag = pong_resources_get("basicFragShader");
	PONG_LOG(vert, PONG_LOG_INFO);
	PONG_LOG(frag, PONG_LOG_INFO);

	return 0;
}

void pong_renderer_drawrect(float x, float y, float w, float h) {
	glBegin(GL_QUADS);
	glVertex2f(x - (w / 2), y - (h / 2));
	glVertex2f(x + (w / 2), y - (h / 2));
	glVertex2f(x + (w / 2), y + (h / 2));
	glVertex2f(x - (w / 2), y + (h / 2));
	glEnd();
}

void pong_renderer_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up renderer...", PONG_LOG_VERBOSE);
	}
}

