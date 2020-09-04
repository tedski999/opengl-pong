#ifndef PONG_FILES_H
#define PONG_FILES_H

int pong_files_init();
const char *pong_files_getDataDirectoryPath();
const char *pong_files_readData(const char *filename);
void pong_files_writeData(const char *filename, const char *data);
void pong_files_appendData(const char *filename, const char *data);
void pong_files_deleteFile(const char *filename);

#endif // PONG_FILES_H

