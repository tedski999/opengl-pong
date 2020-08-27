#ifndef PONG_CORE_H
#define PONG_CORE_H

#define PONG_WINDOW_WIDTH 640
#define PONG_WINDOW_HEIGHT 480
#define PONG_RESOURCE_PATH "data.wad"

#ifdef PONG_GL_DEBUG
#define PONG_OPENGL_VERSION_MAJOR_MIN 4
#define PONG_OPENGL_VERSION_MINOR_MIN 3
#else
#define PONG_OPENGL_VERSION_MAJOR_MIN 3
#define PONG_OPENGL_VERSION_MINOR_MIN 3
#endif

#ifdef PONG_VERBOSE_LOGS
#define PONG_VERBOSE_LOGS 1
#else
#define PONG_VERBOSE_LOGS 0
#endif

#define NSEC_PER_SEC 1000000000

#endif // PONG_CORE_H
