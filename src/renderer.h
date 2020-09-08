#ifndef PONG_RENDERER_H
#define PONG_RENDERER_H

void pong_renderer_init(void);
void pong_renderer_drawrect(float x, float y, float w, float h);
void pong_renderer_clearScreen(void);
void pong_renderer_cleanup(void);

#endif // PONG_RENDERER_H

