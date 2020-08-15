#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_TICK (NSEC_PER_SEC / 60)

static bool is_running;

void window_close_callback(GLFWwindow *context) {
	is_running = false;
}

int main(int argc, char *argv[]) {
	glfwInit();
	GLFWwindow *window = glfwCreateWindow(640, 480, "Pong", NULL, NULL);
	glfwSetWindowCloseCallback(window, window_close_callback);
	glfwMakeContextCurrent(window);

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));

	unsigned int accumulated_time;
	struct timespec current_time, previous_time;
	float angle,p1x,p1y,p2x,p2y,p3x,p3y;
	unsigned int tick_count, draw_count, current_second, artifical_stress;

	is_running = true;
	accumulated_time = 0;
	clock_gettime(CLOCK_MONOTONIC, &previous_time);
	angle = p1x = p1y = p2x = p2y = p3x = p3y = 0.0f;
	tick_count = draw_count = artifical_stress = 0;
	current_second = previous_time.tv_sec;

	do {
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		accumulated_time += ((current_time.tv_sec - previous_time.tv_sec) * NSEC_PER_SEC) + (current_time.tv_nsec - previous_time.tv_nsec);
		previous_time = current_time;

		while (accumulated_time >= NSEC_PER_TICK) {
			accumulated_time -= NSEC_PER_TICK;
			angle += 0.01f;
			p1x = cos(angle + 0.0f); p1y = sin(angle + 0.0f);
			p2x = cos(angle + 2.1f); p2y = sin(angle + 2.1f);
			p3x = cos(angle + 4.2f); p3y = sin(angle + 4.2f);
			tick_count++;
		}

		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(p1x,p1y);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(p2x,p2y);
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex2f(p3x,p3y);
		glEnd();
		glfwSwapBuffers(window);
		usleep(artifical_stress); // simulate rendering stress
		draw_count++;

		if (current_time.tv_sec > current_second) {
			current_second = current_time.tv_sec;
			printf("%i (stress %ius) -> TPS: %i - FPS: %i\n", current_second, artifical_stress, tick_count, draw_count);
			tick_count = draw_count = 0;
			artifical_stress += 1000;
		}
	} while (is_running);
	
	glfwDestroyWindow(window);
	glfwTerminate();
}

