#ifdef PONG_LOGGING

#include "log.h"
#include "core.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#ifdef PONG_LOGGING_FILE
#include <stdlib.h>
#include <string.h>
#if PONG_PLATFORM_WINDOWS
#include <windows.h>
#elif PONG_PLATFORM_LINUX
#include <unistd.h>
#endif
#endif

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

#ifdef PONG_LOGGING_FILE
static char *log_filepath;
#endif

static const char *urgency_labels[PongLogUrgencyCount] = { "VERB", "INFO", "NOTE", "WARN", "ERRR" };
static struct timespec init_time;

int pong_log_internal_init() {
	clock_gettime(CLOCK_MONOTONIC, &init_time);
	time_t now = time(NULL);
	struct tm *time_raw = localtime(&now);
	char time_string[PONG_LOG_TIME_BUF_SIZE];
	strftime(time_string, sizeof (char[PONG_LOG_TIME_BUF_SIZE]), "%Y-%m-%d %H:%M:%S %Z\n", time_raw);
	printf(time_string);

#ifdef PONG_LOGGING_FILE
	// Duplicate code from files.c to remove dependencies in a debugging tool
	int buffer_used, buffer_size = 64;
	do {
		buffer_size *= 2;
		free(log_filepath);
		log_filepath = malloc(sizeof (char) * buffer_size);
#ifdef PONG_PLATFORM_WINDOWS
		buffer_used = GetModuleFileNameA(NULL, log_filepath, buffer_size - 1);
#elif PONG_PLATFORM_LINUX
		buffer_used = readlink("/proc/self/exe", log_filepath, buffer_size - 1);
#endif
		if (buffer_used <= 0) {
			printf("Failed to initialize logging system: Could not locate data directory for log file!");
			free(log_filepath);
			return 1;
		}
	} while (buffer_used >= buffer_size - 2);
	log_filepath[buffer_used] = '\0';
	strrchr(log_filepath, PONG_PATH_DELIMITER)[1] = '\0';
	log_filepath = realloc(log_filepath, sizeof (char) * (strlen(log_filepath) + strlen(PONG_LOG_FILE) + 1));
	strcat(log_filepath, PONG_LOG_FILE);

	FILE *log_file = fopen(log_filepath, "w");
	fprintf(log_file, time_string);
	fclose(log_file);
#endif

	PONG_LOG("Logging initialized!", PONG_LOG_VERBOSE);
	return 0;
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
#ifdef PONG_LOGGING_FILE
	FILE *log_file = fopen(log_filepath, "a");
	fprintf(log_file, log_string);
	fclose(log_file);
#endif
}

void pong_log_internal_cleanup() {
	PONG_LOG("Cleaning up logging...", PONG_LOG_VERBOSE);
#ifdef PONG_LOGGING_FILE
	// TODO: backup log file
	free(log_filepath);
#endif
}

#else

typedef int this_is_not_an_empty_translation_unit;

#endif

