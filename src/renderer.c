// TODO: modern opengl

#include "renderer.h"
#include "log.h"
#include <GL/gl.h>

int pong_renderer_init() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	PONG_LOG("Vendor: %s", PONG_LOG_INFO, glGetString(GL_VENDOR));
	PONG_LOG("Renderer: %s", PONG_LOG_INFO, glGetString(GL_RENDERER));
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

}

