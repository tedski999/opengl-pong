#ifndef PONG_RENDERER_H
#define PONG_RENDERER_H

void pong_renderer_init();
void pong_renderer_drawrect(float x, float y, float w, float h);
void pong_renderer_clearScreen();
void pong_renderer_cleanup();

#endif // PONG_RENDERER_H

