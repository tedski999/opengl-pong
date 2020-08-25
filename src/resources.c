#include "resources.h"
#include "log.h"
#include <zip.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define RESOURCE_PATH "data.wad"

static bool safe_to_clean = false;
static struct zip *zip_archive;
static void *foo; //temp

int pong_resources_init() {
	PONG_LOG("Initializing resource manager...", PONG_LOG_INFO);

	// TODO: method to ensure cwd is located at app root

	PONG_LOG("Opening resource data archive...", PONG_LOG_VERBOSE);
    int err = 0;
    zip_archive = zip_open(RESOURCE_PATH, 0, &err);
	if (err) {
		struct zip_error error;
		zip_error_init_with_code(&error, err);
		PONG_LOG("An error occured while trying to read '%s': %s", PONG_LOG_ERROR, RESOURCE_PATH, zip_error_strerror(&error));
		return 1;
	}

	safe_to_clean = true;
	PONG_LOG("Resource manager initialized!", PONG_LOG_VERBOSE);
	return 0;
}

void pong_resources_load(const char *file_path, const char *resource_id) {
	PONG_LOG("Loading resource at '%s'...", PONG_LOG_VERBOSE, file_path);

	PONG_LOG("Querying resource...", PONG_LOG_VERBOSE);
	struct zip_stat stat;
	zip_stat_init(&stat);
	if (zip_stat(zip_archive, file_path, 0, &stat)) {
		PONG_LOG("An error occured while trying to query requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
		return;
	}

	PONG_LOG("Opening resource...", PONG_LOG_VERBOSE);
	char *data = malloc(sizeof (char) * stat.size + 1);
	struct zip_file *file = zip_fopen(zip_archive, file_path, 0);
	if (!file) {
		PONG_LOG("An error occured while trying to open requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
		return;
	}

	PONG_LOG("Reading resource...", PONG_LOG_VERBOSE);
	zip_int64_t bytes_read;
	zip_int64_t bytes_remaining = stat.size;
	do {
		bytes_read = zip_fread(file, data, stat.size);
		if (bytes_read == -1) {
			PONG_LOG("An error occured while trying to load requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
			return;
		}
	} while (bytes_remaining -= bytes_read);
	zip_fclose(file);
	data[stat.size] = '\0';

	// TODO: add data to resource_map (char*->void* map) with key resource_id
	PONG_LOG("Mapping resource...", PONG_LOG_VERBOSE);
	foo = data; //temp
}

void pong_resources_unload(const char *resource_id) {
	// TODO: remove data from resource_map
}

void *pong_resources_get(const char *resource_id) {
	// TODO: return data from resource_map
	return foo; //temp
}

void pong_resources_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up resource manager...", PONG_LOG_INFO);
		// TODO: free the resource_map
		zip_close(zip_archive);
	}
}

