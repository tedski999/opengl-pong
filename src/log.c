#ifdef PONG_LOGGING

#include "log.h"
#include "core.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#ifdef PONG_FILE_LOGGING
#include <zlib.h>
#if PONG_PLATFORM_WINDOWS
#include <windows.h>
#define mkdir(dir, mode) mkdir(dir)
#elif PONG_PLATFORM_LINUX
#include <sys/stat.h>
#include <unistd.h>
#endif
#define PONG_LOG_DIRECTORY "logs"
#define PONG_LOG_FILE "latest.txt"
#endif

static void pong_log_internal_generateGroupsString();

#ifdef PONG_COLORED_LOGS
static const char *log_colors[PongLogUrgencyCount + 1] = { "\033[2;37m", "\033[0;37m", "\033[1;32m", "\033[1;33m", "\033[7;31m", "\033[0m" };
#else
static const char *log_colors[PongLogUrgencyCount + 1] = { "", "", "", "", "", "" };
#endif

#ifdef PONG_FILE_LOGGING
static char *log_directory_path;
static char *log_file_path;
static char *compressed_log_file_path;
#endif

static const char *urgency_labels[PongLogUrgencyCount] = { "VERB", "INFO", "NOTE", "WARN", "ERRR" };
static const char **group_titles;
static unsigned int group_titles_len;
static char *groups_string;
static struct timespec init_time;

int pong_log_internal_init(void) {
	clock_gettime(CLOCK_MONOTONIC, &init_time);
	time_t now = time(NULL);
	struct tm *time_raw = localtime(&now);
	char *time_string = NULL;
	unsigned int time_string_len = 16; // should need at least 22 chars
	do {
		time_string_len *= 2;
		free(time_string);
		time_string = malloc(sizeof (char) * time_string_len);
		if (!time_string) {
			printf("Could not allocate memory for datetime header message!\n");
			return 1;
		}
	} while (!strftime(time_string, time_string_len, "%Y-%m-%d %H:%M:%S %Z\n", time_raw));
	printf(time_string);

	pong_log_internal_generateGroupsString();

#ifdef PONG_FILE_LOGGING
	int buffer_used, buffer_size = 64;
	do {
		buffer_size *= 2;
		free(log_file_path);
		log_directory_path = malloc(sizeof (char) * buffer_size);
		if (!log_directory_path) {
			printf("Could not allocate memory for log directory path!\n");
			return 1;
		}
#ifdef PONG_PLATFORM_WINDOWS
		buffer_used = GetModuleFileNameA(NULL, log_directory_path, buffer_size - 1);
#elif PONG_PLATFORM_LINUX
		buffer_used = readlink("/proc/self/exe", log_directory_path, buffer_size - 1);
#endif
		if (buffer_used <= 0) {
			printf("Failed to initialize logging system: Could not locate data directory for logging!\n");
			free(log_directory_path);
			return 1;
		}
	} while (buffer_used >= buffer_size - 2);
	log_directory_path[buffer_used] = '\0';
	strrchr(log_directory_path, PONG_PATH_DELIMITER)[1] = '\0';
	int log_directory_path_len = strlen(log_directory_path) + strlen(PONG_LOG_DIRECTORY) + 2;
	char *new_log_directory_path = realloc(log_directory_path, sizeof (char) * log_directory_path_len);
	if (!new_log_directory_path) {
		printf("Could not reallocate memory for log directory path!\n");
		free(log_directory_path);
		return 1;
	}
	log_directory_path = new_log_directory_path;

	strcat(log_directory_path, PONG_LOG_DIRECTORY);
	log_directory_path[log_directory_path_len - 2] = PONG_PATH_DELIMITER;
	log_directory_path[log_directory_path_len - 1] = '\0';

	log_file_path = malloc(sizeof (char) * (strlen(log_directory_path) + strlen(PONG_LOG_FILE) + 1));
	if (!log_file_path) {
		printf("Could not allocate memory for log file path!\n");
		free(log_directory_path);
		return 1;
	}
	strcpy(log_file_path, log_directory_path);
	strcat(log_file_path, PONG_LOG_FILE);

	char *compressed_log_file_name = NULL;
	unsigned int compressed_log_file_name_len = 16; // should only need ~22 chars
	do {
		compressed_log_file_name_len *= 2;
		free(compressed_log_file_name);
		compressed_log_file_name = malloc(sizeof (char) * compressed_log_file_name_len);
		if (!compressed_log_file_name) {
			printf("Could not allocate memory for compressed log file name!\n");
			return 1;
		}
	} while (!strftime(compressed_log_file_name, compressed_log_file_name_len, "%Y-%m-%dT%H-%M-%S.gz", time_raw));

	compressed_log_file_path = malloc(sizeof (char) * (strlen(log_directory_path) + strlen(compressed_log_file_name) + 1));
	if (!compressed_log_file_path) {
		printf("Could not allocate memory for compressed log file path!\n");
		free(log_directory_path);
		free(log_file_path);
		return 1;
	}
	strcpy(compressed_log_file_path, log_directory_path);
	strcat(compressed_log_file_path, compressed_log_file_name);
	free(compressed_log_file_name);

	mkdir(log_directory_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	FILE *log_file = fopen(log_file_path, "w");
	fprintf(log_file, time_string);
	fclose(log_file);

	printf("Log file will be located at '%s'.\n", log_file_path);
	printf("Compressed log file will be located at '%s'.\n", compressed_log_file_path);
#endif

	free(time_string);
	return 0;
}

void pong_log_internal_log(const char *message, enum PongLogUrgency urgency, ...) {
	va_list args;
	va_start(args, urgency);
	pong_log_internal_log_variadic(message, urgency, args);
	va_end(args);
}

void pong_log_internal_log_variadic(const char *message, enum PongLogUrgency urgency, va_list args) {

#ifndef PONG_VERBOSE_LOGS
	if (urgency == PONG_LOG_VERBOSE)
		return;
#endif

	static struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	float time_since_init = (current_time.tv_sec - init_time.tv_sec) + ((float) (current_time.tv_nsec - init_time.tv_nsec) / NSEC_PER_SEC);

	va_list args_copy;
	va_copy(args_copy, args);
	int formatted_message_len = vsnprintf(NULL, 0, message, args_copy) + 1;
	va_end(args_copy);
	char *formatted_message = malloc(sizeof (char) * formatted_message_len);
	vsprintf(formatted_message, message, args);

	int log_string_len = snprintf(NULL, 0, "%.4f %s[%s] %s%s%s\n", time_since_init, log_colors[urgency], urgency_labels[urgency], groups_string, formatted_message, log_colors[PongLogUrgencyCount]) + 1;
	char *log_string = malloc(sizeof (char) * log_string_len);
	sprintf(log_string, "%.4f %s[%s] %s%s%s\n", time_since_init, log_colors[urgency], urgency_labels[urgency], groups_string, formatted_message, log_colors[PongLogUrgencyCount]);

	printf(log_string);
#ifdef PONG_FILE_LOGGING
	mkdir(log_directory_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	FILE *log_file = fopen(log_file_path, "a");
	fprintf(log_file, log_string);
	fclose(log_file);
#endif

	free(formatted_message);
	free(log_string);
}

void pong_log_internal_pushSubgroup(const char *group_title) {
	unsigned int new_group_titles_len = group_titles_len + 1;
	const char **new_group_titles = realloc(group_titles, sizeof (const char *) * new_group_titles_len);
	if (!new_group_titles)
		return;
	group_titles = new_group_titles;
	group_titles[group_titles_len] = group_title;
	group_titles_len = new_group_titles_len;
	pong_log_internal_generateGroupsString();
}

void pong_log_internal_popSubgroup(void) {
	if (group_titles_len) {
		group_titles_len--;
		pong_log_internal_generateGroupsString();
	}
}

void pong_log_internal_clearSubgroups(void) {
	if (group_titles_len) {
		group_titles_len = 0;
		pong_log_internal_generateGroupsString();
	}
}

void pong_log_internal_generateGroupsString(void) {
	if (!group_titles_len) {
		free(groups_string);
		 groups_string = strdup("");
	} else {
		unsigned int new_groups_string_len = 5; // 2 borders + " - "
		for (unsigned int i = 0; i < group_titles_len; i++)
			new_groups_string_len += strlen(group_titles[i]) + 1; // +1 for subgroup seperator and final null terminator
		char *new_groups_string = realloc(groups_string, sizeof (char) * new_groups_string_len);
		if (!new_groups_string)
			return;
		groups_string = new_groups_string;

		char *groups_str_ptr = groups_string;
		*groups_str_ptr++ = '(';
		for (unsigned int i = 0; i < group_titles_len; i++) {
			const char *title_str_ptr = group_titles[i];
			while (*title_str_ptr != '\0')
				*groups_str_ptr++ = *title_str_ptr++;
			*groups_str_ptr++ = '/';
		}
		*--groups_str_ptr = '\0';
		strcat(groups_string, ") - ");
	}
}

void pong_log_internal_cleanup(void) {
	printf("Clearing remaining log subgroups...\n");
	free(group_titles);
	free(groups_string);

#ifdef PONG_FILE_LOGGING
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

