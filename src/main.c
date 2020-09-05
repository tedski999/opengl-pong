#include "pong.h"
#include "log.h"

int main(int argc, char *argv[]) {
	if (PONG_LOG_INIT())
		return 1;

	int exit_code = 0;
	if (!pong_init()) {
		pong_start();
	} else {
		PONG_LOG("Failed to initialize game, aborting...", PONG_LOG_WARNING);
		exit_code = 1;
	}

	pong_cleanup();
	PONG_LOG_CLEANUP();
	return exit_code;
}

