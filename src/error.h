#ifndef PONG_ERROR_H
#define PONG_ERROR_H

#ifdef PONG_LOGGING
#define PONG_ERROR(...) pong_error_internal_error(__VA_ARGS__)
#else
#define PONG_ERROR(...) pong_error_internal_error(NULL)
#endif

void pong_error_internal_error(const char *message, ...);

#endif // PONG_ERROR_H

