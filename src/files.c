#include "files.h"
#include "core.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#if PONG_PLATFORM_WINDOWS
#include <windows.h>
#elif PONG_PLATFORM_LINUX
#include <unistd.h>
#endif

static char *data_directory;

int pong_files_init() {
	PONG_LOG("Initializing file manager...", PONG_LOG_INFO);

	PONG_LOG("Locating data directory...", PONG_LOG_VERBOSE);
	int buffer_used, buffer_size = 64;
	do {
		buffer_size *= 2;
		PONG_LOG("Trying to fit directory path into %i character buffer...", PONG_LOG_VERBOSE, buffer_size);
		free(data_directory);
		data_directory = malloc(sizeof (char) * buffer_size);
#ifdef PONG_PLATFORM_WINDOWS
		buffer_used = GetModuleFileNameA(NULL, data_directory, buffer_size - 1);
#elif PONG_PLATFORM_LINUX
		buffer_used = readlink("/proc/self/exe", data_directory, buffer_size - 1);
#endif
		if (buffer_used <= 0) {
			PONG_LOG("Could not locate data directory!", PONG_LOG_ERROR);
			return 1;
		}
	} while (buffer_used >= buffer_size - 2);
	data_directory[buffer_used] = '\0';
	strrchr(data_directory, PONG_PATH_DELIMITER)[1] = '\0';
	data_directory = realloc(data_directory, sizeof (char) * (strlen(data_directory) + 1));
	PONG_LOG("Directory path fit into %i character buffer.", PONG_LOG_VERBOSE, sizeof (char) * (strlen(data_directory) + 1));

	PONG_LOG("File manager initialized to '%s'.", PONG_LOG_VERBOSE, data_directory);
	return 0;
}

const char *pong_files_getDataDirectoryPath() {
	return (const char *) data_directory;
}

void pong_files_cleanup() {
	PONG_LOG("Cleaning up file manager...", PONG_LOG_INFO);
	free(data_directory);
}

