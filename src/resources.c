#include "resources.h"
#include "core.h"
#include "files.h"
#include "log.h"
#include <zip.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PONG_RESOURCES_MAP_INITIAL_COUNT 8
#define PONG_RESOURCES_MAP_MAX_USED_BUCKET_RATIO 0.8f
#define PONG_RESOURCES_MAP_AUTOMATIC_COUNT_INCREASE 8

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

	// FIXME: c string manip is difficult, fails with path lengths over 256
	char resources_filepath[256];
	strncat(strncpy(resources_filepath, pong_files_getDataDirectoryPath(), 255), PONG_RESOURCES_FILE, 256);

	PONG_LOG("Opening resource data archive at '%s'...", PONG_LOG_VERBOSE, resources_filepath);
	int err = 0;
	zip_archive = zip_open(resources_filepath, 0, &err);
	if (err) {
		struct zip_error error;
		zip_error_init_with_code(&error, err);
		PONG_LOG("An error occurred while trying to open '%s': %s", PONG_LOG_ERROR, resources_filepath, zip_error_strerror(&error));
		return 1;
	}
	safe_to_clean = true;

	PONG_LOG("Initializing resource map...", PONG_LOG_VERBOSE);
	resource_map = calloc(1, sizeof (struct PongResourceMap) * PONG_RESOURCES_MAP_INITIAL_COUNT);
	resource_map_count_max = PONG_RESOURCES_MAP_INITIAL_COUNT;

	PONG_LOG("Resource manager initialized!", PONG_LOG_VERBOSE);
	return 0;
}

void pong_resources_load(const char *file_path, const char *resource_id) {
	PONG_LOG("Loading resource at '%s' as '%s'...", PONG_LOG_VERBOSE, file_path, resource_id);

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
		free(data);
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
	if (++resource_map_count_cur > resource_map_count_max * PONG_RESOURCES_MAP_MAX_USED_BUCKET_RATIO) {
		PONG_LOG("Resource map is over-encumbered (%i/%i buckets used), automatically adding more buckets...", PONG_LOG_WARNING, resource_map_count_cur - 1, resource_map_count_max);
		pong_resources_changeResourceMapCount(PONG_RESOURCES_MAP_AUTOMATIC_COUNT_INCREASE);
	}
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	if (resource_map[hash_index].key) {
		PONG_LOG("Resource ID '%s' already exists, overwriting loaded data with new resource '%s'...", PONG_LOG_WARNING, resource_map[hash_index].key, resource_id);
		pong_resources_internal_deallocateResource(hash_index);
	}
	resource_map[hash_index] = (struct PongResourceMap) { resource_id, data };

	PONG_LOG("Resource '%s' successfully loaded and mapped to '%s'...", PONG_LOG_VERBOSE, file_path, resource_id);
}

void pong_resources_unload(const char *resource_id) {
	PONG_LOG("Deallocating resource '%s'...", PONG_LOG_VERBOSE, resource_id);
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	pong_resources_internal_deallocateResource(hash_index);
}

void *pong_resources_get(const char *resource_id) {
	unsigned int hash_index = pong_resources_internal_getResourceMapHashIndex(resource_id);
	return resource_map[hash_index].data;
}

void pong_resources_changeResourceMapCount(int count_change) {
	if ((int) resource_map_count_max + count_change < resource_map_count_cur) {
		PONG_LOG("Could not change resource map bucket count from %i to %i! The resource map is currently %i/%i full.", PONG_LOG_WARNING,
			resource_map_count_max, (int) resource_map_count_max + count_change, resource_map_count_cur, resource_map_count_max);
		return;
	}

	PONG_LOG("Changing resource map bucket count from %i to %i...", PONG_LOG_VERBOSE, resource_map_count_max, resource_map_count_max + count_change);
	struct PongResourceMap *old_resource_map = resource_map;
	unsigned int old_resource_map_count = resource_map_count_max;
	resource_map_count_max += count_change;
	resource_map = calloc(1, sizeof (struct PongResourceMap) * resource_map_count_max);

	PONG_LOG("Rehashing resource map entries...", PONG_LOG_VERBOSE);
	for (unsigned int old_hash_index = 0; old_hash_index < old_resource_map_count; old_hash_index++) {
		if (old_resource_map[old_hash_index].key) {
			unsigned int new_hash_index = pong_resources_internal_getResourceMapHashIndex(old_resource_map[old_hash_index].key);
			resource_map[new_hash_index] = old_resource_map[old_hash_index];
		}
	}

	free(old_resource_map);
	PONG_LOG("Successfully changed size of resource map!", PONG_LOG_VERBOSE);
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
	const char *key_char = key;
	unsigned int hash_index = 5381;
	while (*key_char++)
		hash_index = ((hash_index << 5) + hash_index) + *key_char;
	hash_index %= resource_map_count_max;
	while (resource_map[hash_index].key && strcmp(resource_map[hash_index].key, key))
		hash_index = (hash_index + 1) % resource_map_count_max;
	return hash_index;
}

void pong_resources_internal_deallocateResource(unsigned int hash_index) {
	free(resource_map[hash_index].data);
	resource_map[hash_index].key = NULL;
	resource_map[hash_index].data = NULL;
	resource_map_count_cur -= 1;
}

