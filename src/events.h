#ifndef PONG_EVENTS_H
#define PONG_EVENTS_H

typedef unsigned int (*PongEventCallback)();

enum PongEventType {
	PONG_EVENT_FOCUS,
	PONG_EVENT_QUIT,
	PongEventTypeCount
};

void pong_events_pushFocusEvent(int is_focused);
void pong_events_pushQuitEvent(void);
void pong_events_addCallback(enum PongEventType event_type, PongEventCallback callback);
void pong_events_removeCallback(enum PongEventType event_type, PongEventCallback callback);
void pong_events_pollEvents(void);
void pong_events_cleanup(void);

#endif // PONG_EVENTS_H

