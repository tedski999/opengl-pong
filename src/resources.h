#ifndef PONG_RESOURCES_H
#define PONG_RESOURCES_H

int pong_resources_init();
void pong_resources_load(const char *file_path, const char *resource_id);
void pong_resources_unload(const char *resource_id);
void *pong_resources_get(const char *resource_id);
void pong_resources_cleanup();

#endif // PONG_RESOURCES_H

