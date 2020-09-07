#include "pong.h"
#include "core.h"
#include "files.h"
#include "events.h"
#include "window.h"
#include "resources.h"
#include "ball.h"
#include "log.h"
#include <time.h>

#define NSEC_PER_TICK NSEC_PER_SEC / 60
#define MAX_NSEC_BEHIND NSEC_PER_SEC / 10

static unsigned int pong_internal_focusCallback(int is_focused);
static unsigned int pong_internal_quitCallback();

static unsigned int is_running;
static struct PongBall *ball;

void pong_init() {
	PONG_LOG("Initializing game...", PONG_LOG_NOTEWORTHY);
	pong_files_init();
	pong_resources_init();
	pong_window_init();
	pong_events_addCallback(PONG_EVENT_FOCUS, &pong_internal_focusCallback);
	pong_events_addCallback(PONG_EVENT_QUIT, &pong_internal_quitCallback);
	ball = pong_ball_create();
	PONG_LOG("Initialization complete!", PONG_LOG_INFO);
}

void pong_start() {
	unsigned int accumulated_time;
	struct timespec current_time, previous_time;
	unsigned int tick_count, draw_count, current_second;

	is_running = 1;
	accumulated_time = 0;
	clock_gettime(CLOCK_MONOTONIC, &previous_time);
	tick_count = draw_count = 0;
	current_second = previous_time.tv_sec;
	PONG_LOG("Entering main game loop...", PONG_LOG_NOTEWORTHY);
	do {
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		accumulated_time += ((current_time.tv_sec - previous_time.tv_sec) * NSEC_PER_SEC) + (current_time.tv_nsec - previous_time.tv_nsec);
		previous_time = current_time;

		if (accumulated_time > MAX_NSEC_BEHIND) {
			PONG_LOG("Can't keep up! Skipping queued update cycles...", PONG_LOG_WARNING);
			accumulated_time = 0;
			pong_window_update();
			pong_events_pollEvents();
		}

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
			PONG_LOG("%itps %ifps", PONG_LOG_INFO, tick_count, draw_count);
			tick_count = draw_count = 0;
		}
	} while (is_running);
	PONG_LOG("Exited main game loop!", PONG_LOG_NOTEWORTHY);
}

void pong_cleanup() {
	PONG_LOG("Cleaning up...", PONG_LOG_NOTEWORTHY);
	pong_ball_destroy(ball);
	pong_events_cleanup();
	pong_window_cleanup();
	pong_resources_cleanup();
	pong_files_cleanup();
}

unsigned int pong_internal_focusCallback(int is_focused) {
	PONG_LOG("Pong focus callback executed! is_focused: %i", PONG_LOG_VERBOSE, is_focused);
	return 1;
}

unsigned int pong_internal_quitCallback() {
	PONG_LOG("Pong quit callback executed!", PONG_LOG_VERBOSE);
	is_running = 0;
	return 1;
}

