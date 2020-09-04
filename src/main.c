#include "pong.h"
#include "log.h"

int main(int argc, char *argv[]) {
	int exit_code = 0;

	PONG_LOG_INIT();
	if (!pong_init()) {
		pong_start();
	} else {
		PONG_LOG("Failed to initialize game, aborting...", PONG_LOG_WARNING);
		exit_code = 1;
	}

	pong_cleanup();
	return exit_code;
}

