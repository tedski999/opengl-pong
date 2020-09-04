#ifndef PONG_LOG_H
#define PONG_LOG_H

#ifdef PONG_LOGGING

#define PONG_LOG_INIT() pong_log_internal_init()
#define PONG_LOG(message, ...) pong_log_internal_log(message, __VA_ARGS__)

enum PongLogUrgency {
	PONG_LOG_VERBOSE,
	PONG_LOG_INFO,
	PONG_LOG_NOTEWORTHY,
	PONG_LOG_WARNING,
	PONG_LOG_ERROR,
	PongLogUrgencyCount
};

void pong_log_internal_init();
void pong_log_internal_log(const char *message, enum PongLogUrgency urgency, ...);

#else

#define PONG_LOG_INIT()
#define PONG_LOG(message, ...)

#endif

#endif // PONG_LOG_H

