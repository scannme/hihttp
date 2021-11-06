#include <stdlib.h>
#include "log.h"
#include "utils.h"
#include <pthread.h>

void assertInSameThread(struct event_loop *eventLoop) {
    if (eventLoop->owner_thread_id != pthread_self()) {
        LOG_ERR("not in the same thread");
        exit(-1);
    }
}

int isInSameThread(struct event_loop *eventLoop) {
    return eventLoop->owner_thread_id == pthread_self();
}
