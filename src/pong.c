#include "pong.h"
#include "events.h"
#include "window.h"
#include "ball.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_TICK (NSEC_PER_SEC / 60)

static bool pong_quitCallback();

static bool is_running;
static struct PongBall *ball;

// TODO: error handling
int pong_init() {
	pong_window_init();
	pong_events_addCallback(PONG_EVENT_QUIT, &pong_quitCallback);
	ball = pong_ball_create();
	return 0;
}

void pong_start() {
	unsigned int accumulated_time;
	struct timespec current_time, previous_time;
	unsigned int tick_count, draw_count, current_second;

	is_running = true;
	accumulated_time = 0;
	clock_gettime(CLOCK_MONOTONIC, &previous_time);
	tick_count = draw_count = 0;
	current_second = previous_time.tv_sec;
	do {

		clock_gettime(CLOCK_MONOTONIC, &current_time);
		accumulated_time += ((current_time.tv_sec - previous_time.tv_sec) * NSEC_PER_SEC) + (current_time.tv_nsec - previous_time.tv_nsec);
		previous_time = current_time;

		while (accumulated_time >= NSEC_PER_TICK) {
			accumulated_time -= NSEC_PER_TICK;
			pong_window_update();
			pong_ball_update(ball);
			pong_events_pollEvents();
			tick_count++;
		}

		pong_ball_draw(ball);
		pong_window_render();
		draw_count++;

		if (current_time.tv_sec > current_second) {
			current_second = current_time.tv_sec;
			printf("%itps %ifps\n", tick_count, draw_count);
			tick_count = draw_count = 0;
		}
	} while (is_running);
}

void pong_cleanup() {
	pong_ball_destroy(ball);
	pong_window_cleanup();
}

bool pong_quitCallback() {
	is_running = false;
	return true;
}

