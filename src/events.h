#ifndef PONG_EVENTS_H
#define PONG_EVENTS_H

typedef unsigned int (*PongEventCallback)();

enum PongEventType {
	PONG_EVENT_FOCUS,
	PONG_EVENT_QUIT,
	PongEventTypeCount
};

void pong_events_addCallback(enum PongEventType event_type, PongEventCallback callback);
void pong_events_removeCallback(enum PongEventType event_type, PongEventCallback callback);
void pong_events_pushEvent(enum PongEventType event_type, ...);
void pong_events_pollEvents(void);
void pong_events_cleanup(void);

#endif // PONG_EVENTS_H

