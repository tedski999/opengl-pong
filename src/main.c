#include "pong.h"
#include "log.h"

int main(int argc, char *argv[]) {
	if (PONG_LOG_INIT())
		return -1;
	pong_init();
	pong_start();
	pong_cleanup();
	PONG_LOG_CLEANUP();
	return 0;
}

