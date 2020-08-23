#include "pong.h"
#include "events.h"
#include "window.h"
#include "ball.h"
#include "log.h"
#include <stdbool.h>
#include <time.h>

#define NSEC_PER_SEC 1000000000
#define NSEC_PER_TICK NSEC_PER_SEC / 60
#define MAX_NSEC_BEHIND NSEC_PER_SEC / 10

static bool pong_quitCallback();

static bool is_running;
static bool safe_to_clean = false;
static struct PongBall *ball;

int pong_init() {
	PONG_LOG_INIT();
	PONG_LOG("Initializing game...", PONG_LOG_INFO);
	if (pong_window_init())
		return 1;
	pong_events_addCallback(PONG_EVENT_QUIT, &pong_quitCallback);
	ball = pong_ball_create();
	safe_to_clean = true;
	PONG_LOG("Initialization complete!", PONG_LOG_VERBOSE);
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
	PONG_LOG("Entering main game loop...", PONG_LOG_INFO);
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
	PONG_LOG("Exited main game loop!", PONG_LOG_INFO);
}

void pong_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up...", PONG_LOG_INFO);
		pong_ball_destroy(ball);
	}

	pong_events_cleanup();
	pong_window_cleanup();
	PONG_LOG_CLEANUP();
}

bool pong_quitCallback() {
	PONG_LOG("Pong quit callback exectuted!", PONG_LOG_VERBOSE);
	is_running = false;
	return true;
}

