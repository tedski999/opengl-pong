#ifdef PONG_LOGGING

#include "log.h"
#include "core.h"
#include "files.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define PONG_LOG_TIME_BUF_SIZE 64
#define PONG_LOG_MESG_BUF_SIZE 256

// TODO: can PONG_LOG_COLORS be preprocessed?
#ifdef PONG_COLORED_LOGS
static const char *PONG_LOG_COLORS[PongLogUrgencyCount] = { "\033[2;37m", "\033[0;37m", "\033[1;32m", "\033[1;33m", "\033[7;31m" };
#define PONG_LOG_RESETCOLOR "\033[0m"
#else
static const char *PONG_LOG_COLORS[PongLogUrgencyCount] = { "", "", "", "", "" };
#define PONG_LOG_RESETCOLOR ""
#endif

static const char *urgency_labels[PongLogUrgencyCount] = { "VERB", "INFO", "NOTE", "WARN", "ERRR" };
static struct timespec init_time;

void pong_log_internal_init() {
	clock_gettime(CLOCK_MONOTONIC, &init_time);
	time_t now = time(NULL);
	struct tm *time_raw = localtime(&now);
	char time_string[PONG_LOG_TIME_BUF_SIZE];
	strftime(time_string, sizeof (char[PONG_LOG_TIME_BUF_SIZE]), "%F %T %Z", time_raw);
	printf("%s\n", time_string);
	pong_files_deleteFile(PONG_LOG_FILE);
	PONG_LOG("Logging initialized!", PONG_LOG_VERBOSE);
}

void pong_log_internal_log(const char *message, enum PongLogUrgency urgency, ...) {
	if (!PONG_VERBOSE_LOGS && urgency == PONG_LOG_VERBOSE)
		return;

	static struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	float time_since_init = (current_time.tv_sec - init_time.tv_sec) + ((float) (current_time.tv_nsec - init_time.tv_nsec) / NSEC_PER_SEC);

	char formatted_message[PONG_LOG_MESG_BUF_SIZE];
	va_list args;
	va_start(args, urgency);
	vsnprintf(formatted_message, sizeof (char[PONG_LOG_MESG_BUF_SIZE]), message, args);
	va_end(args);

	char log_string[PONG_LOG_MESG_BUF_SIZE];
	int log_string_len = snprintf(log_string, sizeof (char[PONG_LOG_MESG_BUF_SIZE]), "%.4f %s[%s] %s%s\n", time_since_init, PONG_LOG_COLORS[urgency], urgency_labels[urgency], formatted_message, PONG_LOG_RESETCOLOR);
	if (log_string_len >= PONG_LOG_MESG_BUF_SIZE) {
		log_string[PONG_LOG_MESG_BUF_SIZE - 2] = '\n';
		char *str = log_string + PONG_LOG_MESG_BUF_SIZE - 5;
		do *str = '.'; while (*++str != '\n');
	}

	printf(log_string);
	pong_files_appendData(PONG_LOG_FILE, log_string);
}

#else

typedef int this_is_not_an_empty_translation_unit;

#endif

