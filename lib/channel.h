#ifndef CHANNEL_H
#define CHANNEL_H

#define EVENT_TIMEOUT 0x01
#define EVENT_READ 0x02 //wait for a socket or FD to become readable
#define EVENT_WRITE 0x4 //wait for a socket or FD to become writeable
#define EVENT_SIGNAL 0x8 //wait for a POSIX signal to be raised

typedef int (*event_read_callback)(void * data);
typedef int (*event_write_callback)(void *data);


struct channel {
    int fd;
    int events;
    event_read_callback eventReadCallback;
    event_write_callback eventWriteCallback;
    void *data; //callback data
};

struct channel * channel_new(int fd, int events, 
        event_read_callback eventReadCallback,
        event_write_callback eventWriteCallback,
        void *data);

int channel_write_event_is_enabled(struct channel *channel);
int channel_write_event_enable(struct channel *channel);
int channel_write_event_disable(struct channel *channel);

#endif
