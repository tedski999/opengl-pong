#include "events.h"
#include "log.h"
#include <stdlib.h>
#include <stdarg.h>

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

struct PongEvent *pong_events_internal_createEvent(enum PongEventType event_type, va_list args);
bool pong_events_internal_executeCallback(PongEventCallback callback, enum PongEventType event_type, union PongEventArguments event_args);

static struct PongEventArray event_queue;
static struct PongEventCallbackArray events_callbacks[PongEventTypeCount];

void pong_events_addCallback(enum PongEventType event_type, PongEventCallback callback) {
	PONG_LOG("Adding callback %p for event type %i...", PONG_LOG_VERBOSE, callback, event_type);
	struct PongEventCallbackArray *event_callbacks = events_callbacks + event_type;
	PongEventCallback *new_callback_array = realloc(event_callbacks->callbacks, sizeof (PongEventCallback) * ++event_callbacks->length);
	if (!new_callback_array) {
		PONG_LOG("Error reallocating memory for callback array!", PONG_LOG_WARNING);
		return;
	}
	event_callbacks->callbacks = new_callback_array;
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
	PongEventCallback *new_callback_array = realloc(event_callbacks->callbacks, sizeof (PongEventCallback) * event_callbacks->length);
	if (!new_callback_array) {
		PONG_LOG("Error reallocating memory for callback array!", PONG_LOG_WARNING);
		return;
	}
	event_callbacks->callbacks = new_callback_array;
}

void pong_events_pushEvent(enum PongEventType event_type, ...) {
	PONG_LOG("Pushing event type %i...", PONG_LOG_VERBOSE, event_type);
	va_list args;
	va_start(args, event_type);
	struct PongEvent *event = pong_events_internal_createEvent(event_type, args);
	va_end(args);
	if (!event)
		return;
	struct PongEvent **new_event_queue = realloc(event_queue.events, sizeof (struct PongEvent *) * ++event_queue.length);
	if (!new_event_queue) {
		PONG_LOG("Could not push event: Unable to allocate memory for event queue!", PONG_LOG_WARNING);
		free(event);
		free(new_event_queue);
		return;
	}
	event_queue.events = new_event_queue;
	event_queue.events[event_queue.length - 1] = event;
}

void pong_events_pollEvents() {
	if (!event_queue.length)
		return;

	PONG_LOG("Processing events (%i queued)...", PONG_LOG_VERBOSE, event_queue.length);
	do {
		struct PongEvent *event = event_queue.events[--event_queue.length];
		PONG_LOG("Handling event type %i...", PONG_LOG_VERBOSE, event->type);
		struct PongEventCallbackArray *event_callbacks = events_callbacks + event->type;
		bool is_handled = false;
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

void pong_events_cleanup() {
	PONG_LOG("Cleaning up events...", PONG_LOG_INFO);
	PONG_LOG("Clearing any remaining events...", PONG_LOG_VERBOSE);
	while (event_queue.length--)
		free(event_queue.events[event_queue.length]);
	free(event_queue.events);
	PONG_LOG("Clearing list of event callbacks...", PONG_LOG_VERBOSE);
	for (unsigned int i = 0; i < PongEventTypeCount; i++)
		free(events_callbacks[i].callbacks);
}

struct PongEvent *pong_events_internal_createEvent(enum PongEventType event_type, va_list args) {
	struct PongEvent *event = malloc(sizeof (struct PongEvent));
	switch (event_type) {
		case PONG_EVENT_FOCUS: *event = (struct PongEvent) { PONG_EVENT_FOCUS, { .window_focus_event = { va_arg(args, int) } } }; break;
		case PONG_EVENT_QUIT:  *event = (struct PongEvent) { PONG_EVENT_QUIT }; break;
		default:
			PONG_LOG("Could not create event: Invalid event type %i!", PONG_LOG_WARNING, event_type);
			free(event);
			event = NULL;
	}
	return event;
}

bool pong_events_internal_executeCallback(PongEventCallback callback, enum PongEventType event_type, union PongEventArguments event_args) {
	PONG_LOG("Executing callback %p...", PONG_LOG_VERBOSE, &callback);
	switch (event_type) {
		case PONG_EVENT_FOCUS: return callback(event_args.window_focus_event.is_focused);
		case PONG_EVENT_QUIT:  return callback();
		default: PONG_LOG("Could not execute callback: Invalid event type %i!", PONG_LOG_WARNING, event_type);
	}
	return false;
}

