#include "events.h"
#include "log.h"
#include <stdlib.h>

// TODO: event arguments
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
	PONG_LOG("Adding callback %p for event type %i...", PONG_LOG_VERBOSE, callback, event_type);
	struct PongEventCallbackArray *event_callbacks = events_callbacks + event_type;
	event_callbacks->callbacks = realloc(event_callbacks->callbacks, sizeof(PongEventCallback) * ++event_callbacks->length);
	event_callbacks->callbacks[event_callbacks->length - 1] = callback;
}

void pong_events_removeCallback(enum PongEventType event_type, PongEventCallback callback) {
	PONG_LOG("Removing callback %p from event type %i...", PONG_LOG_VERBOSE, callback, event_type);
	struct PongEventCallbackArray *event_callbacks = events_callbacks + event_type;
	unsigned int shift, occurences;
	for (unsigned int i = shift = occurences = 0; i < event_callbacks->length; i++) {
		if (event_callbacks->callbacks[i] != callback)
			event_callbacks->callbacks[shift++] = event_callbacks->callbacks[i];
		else
			occurences++;
	}
	event_callbacks->length -= occurences;
	event_callbacks->callbacks = realloc(event_callbacks->callbacks, sizeof (PongEventCallback) * event_callbacks->length);
}

void pong_events_pushEvent(enum PongEventType event_type) {
	PONG_LOG("Pushing event type %i...", PONG_LOG_VERBOSE, event_type);
	event_queue.events = realloc(event_queue.events, sizeof (struct PongEvent) * ++event_queue.length);
	event_queue.events[event_queue.length - 1] = (struct PongEvent) { event_type };
}

void pong_events_pollEvents() {
	if (!event_queue.length)
		return;

	PONG_LOG("Processing events (%i queued)...", PONG_LOG_VERBOSE, event_queue.length);
	do {
		struct PongEvent *event = event_queue.events + --event_queue.length;
		PONG_LOG("Handling event type %i...", PONG_LOG_VERBOSE, event->type);
		struct PongEventCallbackArray *event_callbacks = events_callbacks + event->type;
		for (unsigned int i = 0; i < event_callbacks->length; i++) {
			PONG_LOG("Executing callback %p...", PONG_LOG_VERBOSE, event_callbacks->callbacks + i);
			if (event_callbacks->callbacks[i]()) {
				PONG_LOG("Event type %i handled.", PONG_LOG_VERBOSE, event->type);
				break;
			}
		}
	} while (event_queue.length);
	
	PONG_LOG("All events handled.", PONG_LOG_VERBOSE);
	free(event_queue.events);
	event_queue.events = NULL;
}

void pong_events_cleanup() {
	PONG_LOG("Cleaning up events...", PONG_LOG_INFO);
	free(event_queue.events);
	for (unsigned int i = 0; i < PongEventTypeCount; i++)
		free(events_callbacks[i].callbacks);
}

