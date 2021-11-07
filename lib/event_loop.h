#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <pthread.h>

#include "channel.h"
#include "event_dispatcher.h"

struct channel_element {
    int type;
    struct channel *channel;
    struct channel_element *next;
};

struct event_loop {
    int quit;
    const struct event_dispatcher *eventDispatcher;

    void *event_dispatcher_data;
    struct channel_map *channelMap;

    int is_handle_pending;
    struct channel_element *pending_head;
    struct channel_element *pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int socketPair[2];
    char *thread_name;
};

struct event_loop *event_loop_init();
int event_loop_run(struct event_loop *eventLoop);
#endif
