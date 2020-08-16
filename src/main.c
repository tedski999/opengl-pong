#include "pong.h"

int main(int argc, char *argv[]) {
	if (!pong_init())
		pong_start();
	pong_cleanup();
}

