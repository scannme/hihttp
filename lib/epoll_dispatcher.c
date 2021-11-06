#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>

#include "channel.h"
#include "event_dispatcher.h"
#include "event_loop.h"
#include "log.h"

#define MAXEVENTS 128

typedef struct {
    int event_count;
    int nfds;
    int realloc_copy;
    int efd;
    struct epoll_event *events;
}epoll_dispatcher_data;

static void *epoll_init(struct event_loop *);

void *epoll_init(struct event_loop *eventLoop) {
    epoll_dispatcher_data *epollDispatcherData = malloc(sizeof(epoll_dispatcher_data));

    if (NULL != epollDispatcherData) {
        return NULL;
    }

    epollDispatcherData->event_count = 0;
    epollDispatcherData->nfds = 0;
    epollDispatcherData->realloc_copy = 0;

    epollDispatcherData->efd = epoll_create1(0);

    if (epollDispatcherData->efd == -1) {
        error(1, errno, "epoll create failed");
    }

    epollDispatcherData->events = calloc(MAXEVENTS, sizeof(struct epoll_event));

    if (NULL == epollDispatcherData->events) {
        error(1, errno, "calloc epool dispatcher struct failed");
    }

    return epollDispatcherData;
}

int epoll_add(struct event_loop *event_loop, struct channel *channel1) {
    int fd = channel1->fd;
    int events = 0;
    struct epoll_event event;

    epoll_dispatcher_data *pollDispatchData = (epoll_dispatcher_data *)event_loop->event_dispatcher_data;

    if (channel1->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (channel1->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    event.events =events;
    event.data.fd = fd;

    if (epoll_ctl(pollDispatchData->efd, EPOLL_CTL_ADD, fd, &event)) {
        error(1, errno, "epoll ctl add fd faild");
    }

    return 0;
}

int epoll_del(struct event_loop *event_loop, struct channel *channel1) {
    int fd = channel1->fd;
    int events = 0;
    struct epoll_event event;

    epoll_dispatcher_data *pollDispatchData = (epoll_dispatcher_data *)event_loop->event_dispatcher_data;

    if (channel1->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (channel1->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    event.events =events;
    event.data.fd = fd;

    if (epoll_ctl(pollDispatchData->efd, EPOLL_CTL_DEL, fd, &event)) {
        error(1, errno, "epoll ctl del fd faild");
    }

    return 0;
}

int epoll_update(struct event_loop *event_loop, struct channel *channel1) {
    int fd = channel1->fd;
    int events = 0;
    struct epoll_event event;

    epoll_dispatcher_data *pollDispatchData = (epoll_dispatcher_data *)event_loop->event_dispatcher_data;

    if (channel1->events & EVENT_READ) {
        events = events | EPOLLIN;
    }
    if (channel1->events & EVENT_WRITE) {
        events = events | EPOLLOUT;
    }

    event.events =events;
    event.data.fd = fd;

    if (epoll_ctl(pollDispatchData->efd, EPOLL_CTL_MOD, fd, &event)) {
        error(1, errno, "epoll ctl update fd faild");
    }

    return 0;
}

void epoll_clear(struct event_loop *eventLoop) {
    epoll_dispatcher_data *epollDispatchData = (epoll_dispatcher_data *)eventLoop->event_dispatcher_data;

    free(epollDispatchData->events);
    close(epollDispatchData->efd);
    free(epollDispatchData);
    eventLoop->event_dispatcher_data = NULL;
}

int epoll_dispatch(struct event_loop *eventLoop, struct timeval *timeval) {
    int i, n;
    epoll_dispatcher_data *epollDispatchData = (epoll_dispatcher_data *)eventLoop->event_dispatcher_data;
    
    n = epoll_wait(epollDispatchData->efd, epollDispatchData->events, MAXEVENTS, -1);

    sys_msgx("epoll wait wakeup, %s", eventLoop->thread_name);

    for (i = 0; i < n; i++) {
        if ((epollDispatchData->events[i].events & EPOLLERR) || (
             epollDispatchData->events[i].events & EPOLLHUP)) {
            fprintf(stderr, "epoll error\n");
            close(epollDispatchData->events[i].data.fd);
            continue;
        }
        
        if (epollDispatchData->events[i].events & EPOLLIN) {
            sys_msgx("get message channel fd==%d for read, %s", epollDispatchData->events[i].data.fd, eventLoop->thread_name);
            channel_event_activate(eventLoop, epollDispatchData->events[i].data.fd, EVENT_READ);
        }
        if (epollDispatchData->events[i].events & EPOLLOUT) {
            sys_msgx("get message channel fd==%d for write, %s", epollDispatchData->events[i].data.fd, eventLoop->thread_name);
            channel_event_activate(eventLoop, epollDispatchData->events[i].data.fd, EVENT_WRITE);
        }
    }

    return 0;
}

const struct event_dispatcher epoll_dispatcher =  {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_del,
    epoll_update,
    epoll_dispatch,
    epoll_clear,
}
