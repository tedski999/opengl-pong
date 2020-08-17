// TODO: modern opengl

#include "renderer.h"
#include <GL/gl.h>
#include <stdio.h>

int pong_renderer_init() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
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

