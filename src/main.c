#include "pong.h"
#include "log.h"

int main(int argc, char *argv[]) {
	if (!pong_init())
		pong_start();
	else
		PONG_LOG("Failed to initialize game, aborting...", PONG_LOG_WARNING);
	pong_cleanup();
	return 0;
}

