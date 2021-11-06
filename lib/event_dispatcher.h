#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include "channel.h"

/*event_dispatcher结*/
struct event_dispatcher {
    const char *name;

    void *(*init) (struct event_loop *eventLoop);
    //add a new dispatcher
    int(add)(struct event_loop *eventLoop, struct channel *channel);
    // del a dispatcher
    int (del)(struct event_loop *eventLoop, struct channel *channel);
    //通知dispatcher 更新channel事件
    int(update)(struct event_loop *eventLoop, struct channel *channel);
    int(dispatcher)(struct event_loop *eventLoop, struct timeval *);

    /*清楚数据*/
    void (clear)(struct event_loop *eventLoop);
};

#endif
