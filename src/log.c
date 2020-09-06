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
#include <zlib.h>
#if PONG_PLATFORM_WINDOWS
#include <windows.h>
#define mkdir(dir, mode) mkdir(dir)
#elif PONG_PLATFORM_LINUX
#include <sys/stat.h>
#include <unistd.h>
#endif
#endif

#define PONG_LOG_TIME_BUF_SIZE 128
#define PONG_LOG_MESG_BUF_SIZE 256

#ifdef PONG_COLORED_LOGS
static const char *log_colors[PongLogUrgencyCount + 1] = { "\033[2;37m", "\033[0;37m", "\033[1;32m", "\033[1;33m", "\033[7;31m", "\033[0m" };
#else
static const char *log_colors[PongLogUrgencyCount + 1] = { "", "", "", "", "", "" };
#endif

#ifdef PONG_LOGGING_FILE
#define PONG_LOG_DIRECTORY "logs"
#define PONG_LOG_FILE "latest.txt"
static char *log_directory_path;
static char *log_file_path;
static char *compressed_log_file_path;
#endif

static const char *urgency_labels[PongLogUrgencyCount] = { "VERB", "INFO", "NOTE", "WARN", "ERRR" };
static struct timespec init_time;

int pong_log_internal_init() {
	clock_gettime(CLOCK_MONOTONIC, &init_time);
	time_t now = time(NULL);
	struct tm *time_raw = localtime(&now);
	char time_string[PONG_LOG_TIME_BUF_SIZE];
	strftime(time_string, sizeof (char) * PONG_LOG_TIME_BUF_SIZE, "%Y-%m-%d %H:%M:%S %Z\n", time_raw);
	printf(time_string);

#ifdef PONG_LOGGING_FILE
	// Duplicate code from files.c to remove dependencies in a debugging tool
	int buffer_used, buffer_size = 64;
	do {
		buffer_size *= 2;
		free(log_file_path);
		log_directory_path = malloc(sizeof (char) * buffer_size);
#ifdef PONG_PLATFORM_WINDOWS
		buffer_used = GetModuleFileNameA(NULL, log_directory_path, buffer_size - 1);
#elif PONG_PLATFORM_LINUX
		buffer_used = readlink("/proc/self/exe", log_directory_path, buffer_size - 1);
#endif
		if (buffer_used <= 0) {
			printf("Failed to initialize logging system: Could not locate data directory for logging!");
			free(log_directory_path);
			return 1;
		}
	} while (buffer_used >= buffer_size - 2);
	log_directory_path[buffer_used] = '\0';
	strrchr(log_directory_path, PONG_PATH_DELIMITER)[1] = '\0';
	int log_directory_path_len = strlen(log_directory_path) + strlen(PONG_LOG_DIRECTORY) + 2;
	log_directory_path = realloc(log_directory_path, sizeof (char) * log_directory_path_len);
	strcat(log_directory_path, PONG_LOG_DIRECTORY);
	log_directory_path[log_directory_path_len - 2] = PONG_PATH_DELIMITER;
	log_directory_path[log_directory_path_len - 1] = '\0';

	log_file_path = malloc(sizeof (char) * (strlen(log_directory_path) + strlen(PONG_LOG_FILE) + 1));
	strcpy(log_file_path, log_directory_path);
	strcat(log_file_path, PONG_LOG_FILE);

	char compressed_log_file_name[PONG_LOG_TIME_BUF_SIZE];
	strftime(compressed_log_file_name, sizeof (char) * PONG_LOG_TIME_BUF_SIZE, "%Y-%m-%dT%H-%M-%S.gz", time_raw);
	compressed_log_file_path = malloc(sizeof (char) * (strlen(log_directory_path) + strlen(compressed_log_file_name) + 1));
	strcpy(compressed_log_file_path, log_directory_path);
	strcat(compressed_log_file_path, compressed_log_file_name);

	mkdir(log_directory_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	FILE *log_file = fopen(log_file_path, "w");
	fprintf(log_file, time_string);
	fclose(log_file);

	printf("Log file will be located at '%s'.\n", log_file_path);
	printf("Compressed log file will be located at '%s'.\n", compressed_log_file_path);
#endif

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
	int log_string_len = snprintf(log_string, sizeof (char[PONG_LOG_MESG_BUF_SIZE]), "%.4f %s[%s] %s%s\n", time_since_init, log_colors[urgency], urgency_labels[urgency], formatted_message, log_colors[PongLogUrgencyCount]);
	if (log_string_len >= PONG_LOG_MESG_BUF_SIZE) {
		log_string[PONG_LOG_MESG_BUF_SIZE - 2] = '\n';
		char *str = log_string + PONG_LOG_MESG_BUF_SIZE - 5;
		do *str = '.'; while (*++str != '\n');
	}

	printf(log_string);
#ifdef PONG_LOGGING_FILE
	mkdir(log_directory_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	FILE *log_file = fopen(log_file_path, "a");
	fprintf(log_file, log_string);
	fclose(log_file);
#endif
}

void pong_log_internal_cleanup() {
#ifdef PONG_LOGGING_FILE
	mkdir(log_directory_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	printf("Compressing latest log file to '%s'...\n", compressed_log_file_path);
	FILE *source = fopen(log_file_path, "rb");
	gzFile dest = gzopen(compressed_log_file_path, "wb");
	char buffer[128];
	int bytes_read = 0;
	while ((bytes_read = fread(buffer, 1, sizeof buffer, source)) > 0)
		gzwrite(dest, buffer, bytes_read);
	fclose(source);
	gzclose(dest);
	printf("Done!\n");

	free(log_directory_path);
	free(log_file_path);
	free(compressed_log_file_path);
#endif
}

#else

typedef int this_is_not_an_empty_translation_unit;

#endif

