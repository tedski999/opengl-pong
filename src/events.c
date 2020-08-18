#include "events.h"
#include <stdlib.h>

struct PongEvent {
	enum PongEventType type;
};

struct PongEventArray {
	struct PongEvent *events;
	unsigned int length;
};

struct PongEventCallbackArray {
	PongEventCallback *callbacks;
	unsigned int length;
};

static struct PongEventArray event_queue;
static struct PongEventCallbackArray events_callbacks[PongEventTypeCount];

void pong_events_addCallback(enum PongEventType event_type, PongEventCallback callback) {
	struct PongEventCallbackArray *event_callbacks = events_callbacks + event_type;
	event_callbacks->callbacks = realloc(event_callbacks->callbacks, sizeof(PongEventCallback) * ++event_callbacks->length);
	event_callbacks->callbacks[event_callbacks->length - 1] = callback;
}

// TODO: create pong_events_removeCallback(enum PongEventType event_type, PongEventCallback callback)

void pong_events_pushEvent(enum PongEventType event_type) {
	event_queue.events = realloc(event_queue.events, sizeof (struct PongEvent) * ++event_queue.length);
	event_queue.events[event_queue.length - 1] = (struct PongEvent) { event_type };
}

void pong_events_pollEvents() {
	if (!event_queue.length)
		return;

	do {
		struct PongEventCallbackArray *event_callbacks = events_callbacks + event_queue.events->type;
		for (unsigned int i = 0; i < event_callbacks->length; i++) {
			if (event_callbacks->callbacks[i]())
				break;
		}
	} while (--event_queue.length);
	
	event_queue.events = NULL;
}

void pong_events_cleanup() {
	free(event_queue.events);
}

