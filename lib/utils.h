#ifndef UTILS_H
#define UTILS_H
#include "event_loop.h"

void assertInSameThread(struct event_loop *eventLoop);
int isInSameThread(struct event_loop *eventLoop);

#endif
