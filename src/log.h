#ifndef PONG_LOG_H
#define PONG_LOG_H

#ifdef PONG_LOGGING

#define PONG_LOG_INIT() pong_log_init()
#define PONG_LOG(message, ...) pong_log(message, __VA_ARGS__)
#define PONG_LOG_CLEANUP() pong_log_cleanup()

enum PongLogUrgency {
	PONG_LOG_VERBOSE,
	PONG_LOG_INFO,
	PONG_LOG_NOTEWORTHY,
	PONG_LOG_WARNING,
	PONG_LOG_ERROR,
	PongLogUrgencyCount
};

void pong_log_init();
void pong_log(const char *message, enum PongLogUrgency urgency, ...);
void pong_log_cleanup();

#else

#define PONG_LOG_INIT()
#define PONG_LOG(message, ...)
#define PONG_LOG_CLEANUP()

#endif

#endif // PONG_LOG_H

