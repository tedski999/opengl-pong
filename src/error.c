#include "error.h"
#include "pong.h"
#include "log.h"
#include <stdlib.h>
#include <stdarg.h>

void pong_error_internal_error(const char *message, ...) {
	va_list args;
	va_start(args, message);
	PONG_LOG_VARIADIC(message, PONG_LOG_ERROR, args);
	va_end(args);
	pong_cleanup();
	PONG_LOG_CLEANUP();
	exit(1);
}

