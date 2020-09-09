#include "resources.h"
#include "core.h"
#include "files.h"
#include "log.h"
#include "error.h"
#include <zip.h>
#include <stdlib.h>
#include <string.h>

#define PONG_RESOURCES_MAP_TABLE_INITIAL_BUCKET_COUNT 8
#define PONG_RESOURCES_MAP_TABLE_MAX_USED_BUCKETS_RATIO 0.8f

struct PongResourceMap {
	const char *key;
	void *data;
};

static unsigned int pong_resources_internal_getHashIndex(const char *key);
static struct PongResourceMap *pong_resources_internal_getResourceMap(const char *key);
static struct PongResourceMap *pong_resources_internal_getEmptyResourceMap(const char *key);
static void pong_resources_internal_deleteResourceMap(struct PongResourceMap *map);
static void pong_resources_internal_setBucketCount(unsigned int new_bucket_count);

static struct zip *zip_archive;
static struct PongResourceMap *resource_map_table;
static unsigned int resource_map_table_bucket_count;
static unsigned int resource_map_table_used_bucket_count;

void pong_resources_init(void) {
	PONG_LOG_SUBGROUP_START("Resources");
	PONG_LOG("Initializing resource manager...", PONG_LOG_INFO);

	const char *data_directory = pong_files_getDataDirectoryPath();
	char *resources_filepath = malloc(sizeof (char) * (strlen(data_directory) + strlen(PONG_RESOURCES_FILE) + 1));
	if (!resources_filepath)
		PONG_ERROR("Could not allocate memory for resources file path!");
	strcat(strcpy(resources_filepath, data_directory), PONG_RESOURCES_FILE);

	PONG_LOG("Opening resource data archive at '%s'...", PONG_LOG_VERBOSE, resources_filepath);
	int err = 0;
	zip_archive = zip_open(resources_filepath, 0, &err);
	free(resources_filepath);
	if (err) {
		struct zip_error error;
		zip_error_init_with_code(&error, err);
		PONG_ERROR("An error occurred while trying to open resource data archive: %s", zip_error_strerror(&error));
	}

	PONG_LOG("Initializing resource map table...", PONG_LOG_VERBOSE);
	resource_map_table_bucket_count = PONG_RESOURCES_MAP_TABLE_INITIAL_BUCKET_COUNT;
	resource_map_table = calloc(1, sizeof (struct PongResourceMap) * resource_map_table_bucket_count);
	if (!resource_map_table)
		PONG_ERROR("Could not allocate memory for resource map!");

	PONG_LOG("Resource manager initialized!", PONG_LOG_VERBOSE);
	PONG_LOG_SUBGROUP_END();
}

void pong_resources_load(const char *file_path, const char *resource_id) {
	PONG_LOG_SUBGROUP_START("ResourceLoad");
	PONG_LOG("Loading resource at '%s' as '%s'...", PONG_LOG_INFO, file_path, resource_id);

	PONG_LOG("Querying resource...", PONG_LOG_VERBOSE);
	struct zip_stat stat;
	zip_stat_init(&stat);
	if (zip_stat(zip_archive, file_path, 0, &stat))
		PONG_ERROR("An error occurred while trying to query requested resource '%s': %s", file_path, zip_strerror(zip_archive));

	PONG_LOG("Opening resource...", PONG_LOG_VERBOSE);
	char *data = malloc(sizeof (char) * stat.size + 1);
	if (!data)
		PONG_ERROR("Could not allocate memory for resource!");
	struct zip_file *file = zip_fopen(zip_archive, file_path, 0);
	if (!file) {
		zip_fclose(file);
		free(data);
		PONG_ERROR("An error occurred while trying to open requested resource '%s': %s", file_path, zip_strerror(zip_archive));
	}

	PONG_LOG("Reading resource...", PONG_LOG_VERBOSE);
	zip_int64_t bytes_read;
	zip_int64_t bytes_remaining = stat.size;
	do {
		bytes_read = zip_fread(file, data, stat.size);
		if (bytes_read == -1)
			PONG_ERROR("An error occurred while trying to load requested resource '%s': %s", file_path, zip_strerror(zip_archive));
	} while (bytes_remaining -= bytes_read);
	zip_fclose(file);
	data[stat.size] = '\0';

	PONG_LOG("Mapping resource...", PONG_LOG_VERBOSE);
	struct PongResourceMap *resource_map = pong_resources_internal_getEmptyResourceMap(resource_id);
	*resource_map = (struct PongResourceMap) { resource_id, data };
	resource_map_table_used_bucket_count++;
	
	PONG_LOG("Resource '%s' successfully loaded and mapped to '%s'...", PONG_LOG_VERBOSE, file_path, resource_id);

	if (resource_map_table_used_bucket_count >= resource_map_table_bucket_count * PONG_RESOURCES_MAP_TABLE_MAX_USED_BUCKETS_RATIO) {
		PONG_LOG("Resource map tree is over-encumbered (%i/%i buckets used), doubling bucket count...", PONG_LOG_WARNING, resource_map_table_used_bucket_count, resource_map_table_bucket_count);
		pong_resources_internal_setBucketCount(resource_map_table_bucket_count * 2);
	}

	PONG_LOG_SUBGROUP_END();
}

void pong_resources_unload(const char *resource_id) {
	PONG_LOG_SUBGROUP_START("ResourceUnload");
	PONG_LOG("Deallocating resource '%s'...", PONG_LOG_VERBOSE, resource_id);
	pong_resources_internal_deleteResourceMap(pong_resources_internal_getResourceMap(resource_id));
	PONG_LOG_SUBGROUP_END();
}

void *pong_resources_get(const char *resource_id) {
	PONG_LOG_SUBGROUP_START("ResourceGet");
	struct PongResourceMap *resource_map = pong_resources_internal_getResourceMap(resource_id)->data;
	PONG_LOG_SUBGROUP_END();
	return resource_map;
}

void pong_resources_cleanup(void) {
	PONG_LOG_SUBGROUP_START("Resources");
	PONG_LOG("Cleaning up resource manager...", PONG_LOG_INFO);
	PONG_LOG("Clearing resource map table...", PONG_LOG_VERBOSE);
	struct PongResourceMap *resource_map = resource_map_table;
	while (resource_map_table_bucket_count--)
		free(resource_map++->data);
	free(resource_map_table);
	PONG_LOG("Closing data archive...", PONG_LOG_VERBOSE);
	if (zip_archive)
		zip_close(zip_archive);
	PONG_LOG_SUBGROUP_END();
}

// Using djb2 hashing algorithm because I'm that basic
static unsigned int pong_resources_internal_getHashIndex(const char *key) {
	PONG_LOG_SUBGROUP_START("HashFunc");
	if (!resource_map_table_bucket_count)
		PONG_ERROR("Attempted to find hash index for the key '%s' but the resource map table doesn't even have any buckets!", key);
	const char *key_char = key;
	unsigned int hash_index = 5381;
	while (*key_char++)
		hash_index = ((hash_index << 5) + hash_index) + *key_char;
	PONG_LOG_SUBGROUP_END();
	return hash_index % resource_map_table_bucket_count;
}

static struct PongResourceMap *pong_resources_internal_getResourceMap(const char *key) {
	unsigned int attempt = 0, hash_index = pong_resources_internal_getHashIndex(key);
	while (resource_map_table[hash_index].key && strcmp(resource_map_table[hash_index].key, key)) {
		if (++attempt >= resource_map_table_bucket_count)
			PONG_ERROR("Could not locate resource with ID '%s'!", key);
		hash_index = (hash_index + 1) % resource_map_table_bucket_count;
	}
	if (!resource_map_table[hash_index].data)
		PONG_ERROR("Attempted to get resource with ID '%s' but it's already been unloaded!");
	return resource_map_table + hash_index;
}

static struct PongResourceMap *pong_resources_internal_getEmptyResourceMap(const char *key) {
	unsigned int attempt = 0, hash_index = pong_resources_internal_getHashIndex(key);
	while (resource_map_table[hash_index].data) {
		if (!strcmp(resource_map_table[hash_index].key, key))
			PONG_ERROR("Attempted to overwrite resource ID '%s'!", key);
		if (++attempt >= resource_map_table_bucket_count)
			PONG_ERROR("Could not find available empty resource map for resource with ID '%s'!", key);
		hash_index = (hash_index + 1) % resource_map_table_bucket_count;
	}
	return resource_map_table + hash_index;
}

static void pong_resources_internal_deleteResourceMap(struct PongResourceMap *resource_map) {
	free(resource_map->data);
	resource_map->data = NULL;
	resource_map_table_used_bucket_count--;
}

static void pong_resources_internal_setBucketCount(unsigned int new_bucket_count) {
	PONG_LOG_SUBGROUP_START("SetBucketCount");
	PONG_LOG("Changing max number of resources from %i to %i...", PONG_LOG_INFO, resource_map_table_bucket_count, new_bucket_count);

	struct PongResourceMap *old_resource_map_table = resource_map_table;
	unsigned int old_resource_map_table_bucket_count = resource_map_table_bucket_count;
	resource_map_table_bucket_count = new_bucket_count;
	resource_map_table = calloc(1, sizeof (struct PongResourceMap) * resource_map_table_bucket_count);
	if (!resource_map_table) {
		resource_map_table = old_resource_map_table;
		resource_map_table_bucket_count = old_resource_map_table_bucket_count;
		PONG_ERROR("Could not allocate memory for resource map!");
	}

	PONG_LOG("Rehashing resource map entries...", PONG_LOG_VERBOSE);
	for (struct PongResourceMap *old_resource_map = old_resource_map_table; old_resource_map_table_bucket_count--; old_resource_map++) {
		if (old_resource_map->data) {
			struct PongResourceMap *new_resource_map = pong_resources_internal_getEmptyResourceMap(old_resource_map->key);
			*new_resource_map = *old_resource_map;
		}
	}

	free(old_resource_map_table);
	PONG_LOG("Successfully changed size of resource map table!", PONG_LOG_VERBOSE);
	PONG_LOG_SUBGROUP_END();
}

