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

#define PONG_FILES_DATA_DIR_BUF_SIZE 256

static char data_directory[PONG_FILES_DATA_DIR_BUF_SIZE];

int pong_files_init() {
	PONG_LOG("Initializing file manager...", PONG_LOG_INFO);

	// FIXME: fails with long (>256 char) paths
	PONG_LOG("Locating data directory...", PONG_LOG_VERBOSE);
#ifdef PONG_PLATFORM_WINDOWS
	int data_directory_path_length = GetModuleFileNameA(NULL, data_directory, PONG_FILES_DATA_DIR_BUF_SIZE);
#elif PONG_PLATFORM_LINUX
	int data_directory_path_length = readlink("/proc/self/exe", data_directory, PONG_FILES_DATA_DIR_BUF_SIZE);
#endif
	if (data_directory_path_length <= 0) {
		PONG_LOG("Could not locate data directory!", PONG_LOG_ERROR);
		return 1;
	}
	strrchr(data_directory, PONG_PATH_DELIMITER)[1] = '\0';

	PONG_LOG("File manager initialized to '%s'.", PONG_LOG_VERBOSE, data_directory);
	return 0;
}

const char *pong_files_getDataDirectoryPath() {
	return (const char *) data_directory;
}


// TODO: all these functions below here

const char *pong_files_readData(const char *filename) {
	const char *data = NULL;
	return data;
}

void pong_files_writeData(const char *filename, const char *data) {
}

void pong_files_appendData(const char *filename, const char *data) {
}

void pong_files_deleteFile(const char *filename) {
}

