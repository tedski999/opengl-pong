#include "events.h"
#include "log.h"
#include "error.h"
#include <stdlib.h>

// TODO: each event is as large as the largest event, use pointers to structs?
union PongEventArguments {
	struct { int is_focused; } window_focus_event;
};

struct PongEvent {
	enum PongEventType type;
	union PongEventArguments arguments;
};

struct PongEventArray {
	struct PongEvent **events;
	unsigned int length;
};

struct PongEventCallbackArray {
	PongEventCallback *callbacks;
	unsigned int length;
};

static void pong_events_internal_pushEvent(struct PongEvent event);
static unsigned int pong_events_internal_executeCallback(PongEventCallback callback, enum PongEventType event_type, union PongEventArguments event_args);

static struct PongEventArray event_queue;
static struct PongEventCallbackArray events_callbacks[PongEventTypeCount];

void pong_events_pushFocusEvent(int is_focused) {
	struct PongEvent event = (struct PongEvent) { PONG_EVENT_FOCUS, { .window_focus_event = { is_focused } } };
	pong_events_internal_pushEvent(event);
}

void pong_events_pushQuitEvent(void) {
	struct PongEvent event = (struct PongEvent) { PONG_EVENT_QUIT };
	pong_events_internal_pushEvent(event);
}

void pong_events_addCallback(enum PongEventType event_type, PongEventCallback callback) {
	PONG_LOG("Adding callback %p for event type %i...", PONG_LOG_VERBOSE, callback, event_type);
	struct PongEventCallbackArray *event_callbacks = events_callbacks + event_type;
	unsigned int new_callback_array_len = event_callbacks->length + 1;
	PongEventCallback *new_callback_array = realloc(event_callbacks->callbacks, sizeof (PongEventCallback) * new_callback_array_len);
	if (!new_callback_array)
		PONG_ERROR("Could not reallocate memory for event callbacks!");
	event_callbacks->callbacks = new_callback_array;
	event_callbacks->callbacks[event_callbacks->length] = callback;
	event_callbacks->length = new_callback_array_len;
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
}

void pong_events_pollEvents(void) {
	if (!event_queue.length)
		return;

	PONG_LOG("Processing events (%i queued)...", PONG_LOG_VERBOSE, event_queue.length);
	do {
		struct PongEvent *event = event_queue.events[--event_queue.length];
		PONG_LOG("Handling event type %i...", PONG_LOG_VERBOSE, event->type);
		struct PongEventCallbackArray *event_callbacks = events_callbacks + event->type;
		unsigned int is_handled = 0;
		for (unsigned int i = 0; !is_handled && i < event_callbacks->length; i++)
			is_handled = pong_events_internal_executeCallback(event_callbacks->callbacks[i], event->type, event->arguments);
		if (is_handled)
			PONG_LOG("Event was handled.", PONG_LOG_VERBOSE);
		else
			PONG_LOG("Event was not handled.", PONG_LOG_VERBOSE);
		free(event);
	} while (event_queue.length);
	
	PONG_LOG("All events processed.", PONG_LOG_VERBOSE);
	free(event_queue.events);
	event_queue.events = NULL;
}

void pong_events_cleanup(void) {
	PONG_LOG("Cleaning up events...", PONG_LOG_INFO);
	PONG_LOG("Clearing any remaining events...", PONG_LOG_VERBOSE);
	while (event_queue.length--)
		free(event_queue.events[event_queue.length]);
	free(event_queue.events);
	PONG_LOG("Clearing list of event callbacks...", PONG_LOG_VERBOSE);
	for (unsigned int i = 0; i < PongEventTypeCount; i++)
		free(events_callbacks[i].callbacks);
}

static void pong_events_internal_pushEvent(struct PongEvent event_data) {
	PONG_LOG("Pushing event type %i...", PONG_LOG_VERBOSE, event_data.type);
	unsigned int new_event_queue_len = event_queue.length + 1;
	struct PongEvent **new_event_queue = realloc(event_queue.events, sizeof (struct PongEvent *) * new_event_queue_len);
	struct PongEvent *event = malloc(sizeof (struct PongEvent));
	if (!new_event_queue || !event)
		PONG_ERROR("Could not allocate memory for new event!");
	*event = event_data;
	event_queue.events = new_event_queue;
	event_queue.events[event_queue.length] = event;
	event_queue.length = new_event_queue_len;
}

static unsigned int pong_events_internal_executeCallback(PongEventCallback callback, enum PongEventType event_type, union PongEventArguments event_args) {
	PONG_LOG("Executing callback %p...", PONG_LOG_VERBOSE, &callback);
	switch (event_type) {
		case PONG_EVENT_FOCUS: return callback(event_args.window_focus_event.is_focused);
		case PONG_EVENT_QUIT:  return callback();
		default: PONG_ERROR("Attempted to execute callback for invalid event type %i!", event_type);
	}
	return 0;
}

