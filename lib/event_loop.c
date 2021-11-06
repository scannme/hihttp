#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "channel.h"
#include "event_dispatcher.h"
#include "log.h"
#include "channel_map.h"
#include "event_loop.h"
#include "utils.h"

int event_loop_handle_pending_add(struct event_loop *eventLoop, int fd,
                                 struct channel * channel) {
    sys_msgx("add channel fd == %d, %d", fd, eventLoop->thread_name);
    struct channel_map *map = eventLoop->channelMap;

    if (fd < 0) {
        return 0;
    }

    if (fd >= map->nentries) {
        if (map_make_space(map, fd, sizeof(struct channel *)) == -1) {
            return -1;
        }
    } 

    if ((map)->entries[fd] == NULL) {
        map->entries[fd] = channel;
        //add channel
        const struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
        eventDispatcher->add(eventLoop, channel);
        return 1;
    }

    return 0;
}

int event_loop_handle_pending_remove(struct event_loop *eventLoop, int fd,
        struct channel *channel1) {
    int retval;
    const struct event_dispatcher *eventDispatcher;
    struct channel_map *map = eventLoop->channelMap;
    assert(fd == channel1->fd);

    if (fd < 0) {
        return 0;
    }
    if (fd >= map->nentries) {
        return -1;
    }
    
    struct channel *channel2 = map->entries[fd];
    eventDispatcher =  eventLoop->eventDispatcher;
    
    if (eventDispatcher->del(eventLoop, channel2) == -1) {
        retval = -1;
    } else {
        retval = 1;
    }

    map->entries[fd] = NULL;
    return retval;
}

int event_loop_handle_pending_update(struct event_loop *eventLoop, int fd,
                    struct channel *channel) {
    sys_msgx("update channel fd == %d, %s", fd, eventLoop->thread_name);
    struct channel_map *map = eventLoop->channelMap;

    if (fd < 0) {
        return 0;
    }

    if ((map)->entries[fd] == NULL) {
        return -1;
    }

    //update channel
    const  struct event_dispatcher *eventDispatcher = eventLoop->eventDispatcher;
    eventDispatcher->update(eventLoop, channel);

    return 0;
}

int channel_event_active(struct event_loop *eventLoop, int fd, int revents) {
    sys_msgx("active channel fd == %d, revents=%d,%s", fd, revents, eventLoop->thread_name);

    struct channel_map *map = eventLoop->channelMap;

    if (fd < 0) {
        return 0;
    }
    if (fd >= map->nentries)
        return (-1);

    struct channel *channel = map->entries[fd];
    assert(fd == channel->fd);

    if (revents & (EVENT_READ)) {
        if (channel->eventReadCallback) {
            channel->eventReadCallback(channel->data);
        }
    }

    if (revents & (EVENT_WRITE)) {
        if (channel->eventWriteCallback) {
            channel->eventWriteCallback(channel->data);
        }
    }

    return 0;
}
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
            event_loop_handle_pending_add(eventLoop, fd, channel);
        } else if (channelElement->type == 2) {
            event_loop_handle_pending_remove(eventLoop, fd, channel);
        }else if(channelElement->type == 3) {
            event_loop_handle_pending_update(eventLoop, fd, channel);
        }
        
        channelElement = channelElement->next;
    }

    eventLoop->pending_head = eventLoop->pending_tail= NULL;
    eventLoop->is_handle_pending = 0;

    pthread_mutex_unlock(&eventLoop->mutex);

    return 0;
}


void event_loop_channel_buffer_nolock(struct event_loop *eventLoop, int fd,
        struct channel *channel1, int type) {
    //add channel into the pending list
    struct channel_element *channelElement = malloc(sizeof(struct channel_element));
    if (NULL == channelElement){
        return;
    }
    channelElement->channel = channel1;
    channelElement->type = type;
    channelElement->next =NULL;

    if (eventLoop->pending_head == NULL) {
        eventLoop->pending_head = eventLoop->pending_tail = channelElement;
    } else {
        eventLoop->pending_tail->next = channelElement;
        eventLoop->pending_tail= channelElement;
    }

    return;
}

void event_loop_wakeup(struct event_loop *eventLoop) {
    char one = 'a';
    ssize_t n = write(eventLoop->socketPair[0], &one, sizeof(one));
    if (n != sizeof one) {
        LOG_ERR("wakeup event loop thread  failed");
    }
}

int event_loop_do_channel_event(struct event_loop *eventLoop, int fd, 
                    struct channel * channel1, int type) {
    //get the lock
    pthread_mutex_lock(&eventLoop->mutex);
    assert(eventLoop->is_handle_pending);
    event_loop_channel_buffer_nolock(eventLoop, fd, channel1, type);
    pthread_mutex_unlock(&eventLoop->mutex);

    if (!isInSameThread(eventLoop)) {
        event_loop_wakeup(eventLoop);
    }else {
        event_loop_handle_pending_channel(eventLoop);
    }

}
    

int event_loop_add_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
    return event_loop_do_channel_event(eventLoop, fd, channel1, 1);
}
int event_loop_remove_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
    return event_loop_do_channel_event(eventLoop, fd, channel1, 2);
}
int event_loop_update_channel_event(struct event_loop *eventLoop, int fd, struct channel *channel1)
{
    return event_loop_do_channel_event(eventLoop, fd, channel1, 3);
}


int handleWakeup(void *data) {
    char one;
    size_t n;
    struct event_loop *eventLoop = (struct event_loop *)data;

    n = read(eventLoop->socketPair[1], &one, sizeof(one));

    if (n != sizeof(one)) {
        LOG_ERR("handleWakeup failed");
    }

    sys_msgx("wakeup, %s",eventLoop->thread_name);
}

struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *eventLoop = (struct event_loop *)malloc(sizeof(struct event_loop));

    pthread_mutex_init(&eventLoop->mutex, NULL);
    pthread_cond_init(&eventLoop->cond, NULL);

    if (thread_name != NULL) {
        eventLoop->thread_name = thread_name;
    } else {
        eventLoop->thread_name = "main thread";
    }

    eventLoop->quit = 0;
    eventLoop->channelMap = malloc(sizeof(struct channel_map));
    map_init(eventLoop->channelMap);
#ifdef EPOLL_ENABLE
    sys_msgx("set epoll as dispathcher, %s", eventLoop->thread_name);
    eventLoop->eventDispatcher = &epoll_dispathcer;
#else
    sys_msgx("set poll as dispatcher, %s", eventLoop->thread_name);
    //eventLoop->eventDispatcher = &poll_dispatcher;
#endif
    eventLoop->event_dispatcher_data = eventLoop->eventDispatcher->init(eventLoop);

    //add the socketfd to event
    eventLoop->owner_thread_id = pthread_self();
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, eventLoop->socketPair) < 0) {
        LOG_ERR("socketpair set fialed");
    }
    eventLoop->is_handle_pending = 0;
    eventLoop->pending_head = NULL;
    eventLoop->pending_tail = NULL;

    struct channel *channel = channel_new(eventLoop->socketPair[1], EVENT_READ,
            handleWakeup, NULL, eventLoop);
    event_loop_add_channel_event(eventLoop, eventLoop->socketPair[1], channel);
}

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

int event_loop_run(struct event_loop *eventLoop) {
    assert(eventLoop != NULL);

    const struct event_dispatcher *dispatcher = eventLoop->eventDispatcher;

    if (eventLoop->owner_thread_id != pthread_self()) {
        exit(1);
    }
    sys_msgx("event loop run, %s", eventLoop->thread_name);
    struct timeval timeval;
    timeval.tv_sec = 1;

    while (!eventLoop->quit) {
        dispatcher->dispatcher(eventLoop, &timeval);
        event_loop_handle_pending_channel(eventLoop);
    }

    sys_msgx("event loop end, %s", eventLoop->thread_name);
    return 0;
}
