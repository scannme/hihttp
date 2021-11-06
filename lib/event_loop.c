#include <assert.h>

#include "channel.h"
#include "log.h"
#include "event_loop.h"

// in the i/o thread
int event_loop_handle_pending_channel(struct event_loop * eventLoop) {

    //get the lock
    pthread_mutex_lock(&eventLoop->mutex);
    eventLoop->is_handle_pending = 1;

    struct channel_element *channelElement = eventLoop->pending_head;
    while (channelElement != NULL) {
        //save into event_map
        struct channel *channel = channelElement->channel;
        int fd = channel->fd;

        if (channelElement->type == 1) {
        }
    }
}

int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd,
                                 struct channel * channel) {
    sys_msgx("add channel fd == %d, %d", fd, eventLoop->thread_name);
    
    struct channel_map *map = eventLoop->channelMap;
    
}

struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *eventLoop = malloc(sizeof(struct event_loop));

    pthread_mutex_init(&eventLoop->mutex, NULL);
    pthread_cond_init(&eventLoop->cond, NULL);
}

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}
