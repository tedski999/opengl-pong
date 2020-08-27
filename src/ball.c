#include "ball.h"
#include "core.h"
#include "renderer.h"
#include "log.h"
#include <stdlib.h>

struct PongBall {
	float xpos, ypos, xsize, ysize, xvel, yvel;
};

struct PongBall *pong_ball_create() {
	PONG_LOG("Creating new ball...", PONG_LOG_VERBOSE);
	struct PongBall *ball = malloc(sizeof (struct PongBall));
	*ball = (struct PongBall) { 0.f, 0.f, 10.f, 10.f, 1.f, 2.f };
	PONG_LOG("Initialized ball at %p!", PONG_LOG_VERBOSE, ball);
	return ball;
}

void pong_ball_update(struct PongBall *ball) {
	ball->xpos += ball->xvel;
	ball->ypos += ball->yvel;
	if (ball->xpos > PONG_WINDOW_WIDTH / 2.f) ball->xpos = -ball->xpos;
	if (ball->ypos > PONG_WINDOW_HEIGHT / 2.f) ball->ypos = -ball->ypos;
}

void pong_ball_draw(struct PongBall *ball) {
	pong_renderer_drawrect(ball->xpos, ball->ypos, ball->xsize, ball->ysize);
}

void pong_ball_destroy(struct PongBall *ball) {
	PONG_LOG("Destroying ball at %p...", PONG_LOG_VERBOSE, ball);
	free(ball);
}

