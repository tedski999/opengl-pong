#include "ball.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>

struct PongBall {
	float xpos, ypos, xsize, ysize, xvel, yvel;
};

struct PongBall *pong_ball_create() {
	struct PongBall *ball = malloc(sizeof ball);
	ball->xpos = 0.f;
	ball->ypos = 0.f;
	ball->xsize = 0.1f;
	ball->ysize = 0.1f;
	ball->xvel = 0.01f;
	ball->yvel = 0.0025f;
	return ball;
}

void pong_ball_update(struct PongBall *ball) {
	ball->xpos += ball->xvel;
	ball->ypos += ball->yvel;

	if (ball->xpos > 1.f) ball->xpos = -ball->xpos;
	if (ball->ypos > 1.f) ball->ypos = -ball->ypos;
}

void pong_ball_draw(struct PongBall *ball) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_TRIANGLES);

	glVertex2f(ball->xpos - (ball->xsize / 2), ball->ypos - (ball->ysize / 2));
	glVertex2f(ball->xpos + (ball->xsize / 2), ball->ypos - (ball->ysize / 2));
	glVertex2f(ball->xpos - (ball->xsize / 2), ball->ypos + (ball->ysize / 2));

	glVertex2f(ball->xpos + (ball->xsize / 2), ball->ypos + (ball->ysize / 2));
	glVertex2f(ball->xpos - (ball->xsize / 2), ball->ypos + (ball->ysize / 2));
	glVertex2f(ball->xpos + (ball->xsize / 2), ball->ypos - (ball->ysize / 2));

	glEnd();
}

void pong_ball_destroy(struct PongBall *ball) {
	free(ball);
}

