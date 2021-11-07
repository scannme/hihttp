#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include "event_loop.h"
#include "event_loop_thread.h"

struct thread_pool {
    //创建thread_pool的主线程
    struct event_loop *mainLoop;
    //是否已经启动 
    int started;
    int thread_number;
    struct event_loop_thread *eventLoopThreads;

    int position; //the location of the thread in the array,
};

struct thread_pool *thread_pool_new(struct event_loop *mainLoop, int threadNumber);
void thread_pool_start(struct thread_pool *);
struct event_loop *thread_pool_get_loop(struct thread_pool *);
#endif
