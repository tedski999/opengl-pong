#ifndef PONG_LOG_H
#define PONG_LOG_H

#ifdef PONG_LOGGING

#define PONG_LOG_INIT() pong_log_internal_init()
#define PONG_LOG(message, ...) pong_log_internal_log(message, __VA_ARGS__)
#define PONG_LOG_SUBGROUP_START(group_title) pong_log_internal_pushSubgroup(group_title)
#define PONG_LOG_SUBGROUP_END() pong_log_internal_popSubgroup()
#define PONG_LOG_CLEAR_SUBGROUPS() pong_log_internal_clearSubgroups()
#define PONG_LOG_CLEANUP() pong_log_internal_cleanup()

enum PongLogUrgency {
	PONG_LOG_VERBOSE,
	PONG_LOG_INFO,
	PONG_LOG_NOTEWORTHY,
	PONG_LOG_WARNING,
	PONG_LOG_ERROR,
	PongLogUrgencyCount
};

int pong_log_internal_init();
void pong_log_internal_log(const char *message, enum PongLogUrgency urgency, ...);
void pong_log_internal_pushSubgroup(const char *group_title);
void pong_log_internal_popSubgroup();
void pong_log_internal_clearSubgroups();
void pong_log_internal_cleanup();

#else

#define PONG_LOG_INIT() 0
#define PONG_LOG(message, ...)
#define PONG_LOG_SUBGROUP_START(group_title)
#define PONG_LOG_SUBGROUP_END()
#define PONG_LOG_CLEAR_SUBGROUPS()
#define PONG_LOG_CLEANUP()

#endif

#endif // PONG_LOG_H

