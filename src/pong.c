#include "pong.h"
#include "ball.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <GLFW/glfw3.h>

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_TICK (NSEC_PER_SEC / 60)

static bool is_running;
static GLFWwindow *window;
static struct PongBall *ball;

void window_close_callback(GLFWwindow *context) {
	is_running = false;
}

int pong_init() {
	glfwInit();
	window = glfwCreateWindow(640, 480, "Pong", NULL, NULL);
	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwMakeContextCurrent(window);

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));

	ball = pong_ball_create();

	return 0;
}

void pong_start() {
	unsigned int accumulated_time;
	struct timespec current_time, previous_time;
	unsigned int tick_count, draw_count, current_second, artifical_stress;

	is_running = true;
	accumulated_time = 0;
	clock_gettime(CLOCK_MONOTONIC, &previous_time);
	tick_count = draw_count = artifical_stress = 0;
	current_second = previous_time.tv_sec;
	do {

		clock_gettime(CLOCK_MONOTONIC, &current_time);
		accumulated_time += ((current_time.tv_sec - previous_time.tv_sec) * NSEC_PER_SEC) + (current_time.tv_nsec - previous_time.tv_nsec);
		previous_time = current_time;

		while (accumulated_time >= NSEC_PER_TICK) {
			accumulated_time -= NSEC_PER_TICK;
			glfwPollEvents();
			pong_ball_update(ball);
			tick_count++;
		}

		pong_ball_draw(ball);
		glfwSwapBuffers(window);
		usleep(artifical_stress); // simulate rendering stress
		draw_count++;

		if (current_time.tv_sec > current_second) {
			current_second = current_time.tv_sec;
			printf("%ims draw stress -> %itps %ifps\n", artifical_stress / 1000, tick_count, draw_count);
			tick_count = draw_count = 0;
			artifical_stress += 1000;
		}
	} while (is_running);
}

void pong_cleanup() {
	pong_ball_destroy(ball);
	glfwDestroyWindow(window);
	glfwTerminate();
}

