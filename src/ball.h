#ifndef PONG_BALL_H
#define PONG_BALL_H

struct PongBall;

struct PongBall *pong_ball_create();
void pong_ball_update(struct PongBall *ball);
void pong_ball_draw(struct PongBall *ball);
void pong_ball_destroy(struct PongBall *ball);

#endif // PONG_BALL_H

