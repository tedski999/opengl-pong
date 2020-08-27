#include "resources.h"
#include "core.h"
#include "log.h"
#include <zip.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#define INITIAL_MAX_RESOURCES_IN_MAP 8

struct PongResourceMap {
	const char *key;
	void *data;
};

static unsigned int pong_resources_internal_getResourceMapHashIndex(const char *key);
static void pong_resources_internal_deallocateResource(unsigned int hash_index);

static bool safe_to_clean = false;
static struct zip *zip_archive;
static struct PongResourceMap *resource_map;
static unsigned int resource_map_count_cur;
static unsigned int resource_map_count_max;

int pong_resources_init() {
	PONG_LOG("Initializing resource manager...", PONG_LOG_INFO);

	// TODO: method to ensure cwd is located at app root

	PONG_LOG("Opening resource data archive...", PONG_LOG_VERBOSE);
    int err = 0;
    zip_archive = zip_open(PONG_RESOURCE_PATH, 0, &err);
	if (err) {
		struct zip_error error;
		zip_error_init_with_code(&error, err);
		PONG_LOG("An error occurred while trying to read '%s': %s", PONG_LOG_ERROR, PONG_RESOURCE_PATH, zip_error_strerror(&error));
		return 1;
	}
	safe_to_clean = true;

	PONG_LOG("Initializing resource map...", PONG_LOG_VERBOSE);
	resource_map = calloc(1, sizeof (struct PongResourceMap) * INITIAL_MAX_RESOURCES_IN_MAP);
	resource_map_count_max = INITIAL_MAX_RESOURCES_IN_MAP;

	PONG_LOG("Resource manager initialized!", PONG_LOG_VERBOSE);
	return 0;
}

void pong_resources_load(const char *file_path, const char *resource_id) {
	PONG_LOG("Loading resource at '%s'...", PONG_LOG_VERBOSE, file_path);

	PONG_LOG("Querying resource...", PONG_LOG_VERBOSE);
	struct zip_stat stat;
	zip_stat_init(&stat);
	if (zip_stat(zip_archive, file_path, 0, &stat)) {
		PONG_LOG("An error occurred while trying to query requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
		return;
	}

	PONG_LOG("Opening resource...", PONG_LOG_VERBOSE);
	char *data = malloc(sizeof (char) * stat.size + 1);
	struct zip_file *file = zip_fopen(zip_archive, file_path, 0);
	if (!file) {
		PONG_LOG("An error occurred while trying to open requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
		return;
	}

	PONG_LOG("Reading resource...", PONG_LOG_VERBOSE);
	zip_int64_t bytes_read;
	zip_int64_t bytes_remaining = stat.size;
	do {
		bytes_read = zip_fread(file, data, stat.size);
		if (bytes_read == -1) {
			PONG_LOG("An error occurred while trying to load requested resource '%s': %s", PONG_LOG_WARNING, file_path, zip_strerror(zip_archive));
			return;
		}
	} while (bytes_remaining -= bytes_read);
	zip_fclose(file);
	data[stat.size] = '\0';

	PONG_LOG("Mapping resource...", PONG_LOG_VERBOSE);
	resource_map_count_cur += 1;
	if (resource_map_count_cur > resource_map_count_max)
		pong_resources_increaseResourceMapMaxCount(resource_map_count_max);
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	if (resource_map[hash_index].key) {
		PONG_LOG("Resource ID '%s' already exists, overwriting loaded data...", PONG_LOG_WARNING, resource_id);
		pong_resources_internal_deallocateResource(hash_index);
	}
	resource_map[hash_index] = (struct PongResourceMap) { resource_id, data };

	PONG_LOG("Resource '%s' successfully loaded and mapped to '%s'...", PONG_LOG_VERBOSE, file_path, resource_id);
}

// FIXME: overtime the bucket count may increase but never decrease.
// maybe automatically shrink resource_map if deemed appropriate?
void pong_resources_unload(const char *resource_id) {
	PONG_LOG("Deallocating resource '%s'...", PONG_LOG_VERBOSE, resource_id);
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	pong_resources_internal_deallocateResource(hash_index);
}

void *pong_resources_get(const char *resource_id) {
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	return resource_map[hash_index].data;
}

void pong_resources_increaseResourceMapMaxCount(unsigned int count_increase) {
	PONG_LOG("Increasing resource map bucket count from %i to %i...", PONG_LOG_VERBOSE, resource_map_count_max, resource_map_count_max + count_increase);
	struct PongResourceMap *old_resource_map = resource_map;
	unsigned int old_resource_map_count = resource_map_count_max;
	resource_map_count_max += count_increase;
	resource_map = calloc(1, sizeof (struct PongResourceMap) * resource_map_count_max);

	PONG_LOG("Rehashing resource map entries...", PONG_LOG_VERBOSE);
	for (unsigned int old_hash_index = 0; old_hash_index < old_resource_map_count; old_hash_index++) {
		if (old_resource_map[old_hash_index].key) {
			unsigned int new_hash_index = pong_resources_internal_getResourceMapHashIndex(old_resource_map[old_hash_index].key);
			resource_map[new_hash_index] = old_resource_map[old_hash_index];
		}
	}

	free(old_resource_map);
	PONG_LOG("Successfully increased size of resource map!", PONG_LOG_VERBOSE);
}

void pong_resources_cleanup() {
	if (safe_to_clean) {
		PONG_LOG("Cleaning up resource manager...", PONG_LOG_INFO);
		for (int hash_index = 0; hash_index < resource_map_count_max; hash_index++)
			pong_resources_internal_deallocateResource(hash_index);
		free(resource_map);
		zip_close(zip_archive);
	}
}

// Using djb2 hashing algorithm because I'm that basic
unsigned int pong_resources_internal_getResourceMapHashIndex(const char *key) {
	unsigned int char_code, hash_index = 5381;
	while ((char_code = *key++))
		hash_index = ((hash_index << 5) + hash_index) + char_code;
	hash_index %= resource_map_count_max;
	while (resource_map[hash_index].key && !strcmp(resource_map[hash_index].key, key))
		hash_index = (hash_index + 1) % resource_map_count_max;
	return hash_index;
}

void pong_resources_internal_deallocateResource(unsigned int hash_index) {
	free(resource_map[hash_index].data);
	resource_map[hash_index].key = NULL;
	resource_map[hash_index].data = NULL;
}

