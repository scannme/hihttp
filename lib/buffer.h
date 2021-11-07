#ifndef BUFFER_H
#define BUFFER_H

#define INIT_BUFFER_SIZE 65536
//数据缓冲区

struct buffer {
    char *data;
    int readIndex;
    int writeIndex;
    int total_size;
};

struct buffer *buffer_new();

void buffer_free(struct buffer *buffer);

int buffer_writeable_size(struct buffer *buffer);

int buffer_readable_size(struct buffer *buffer);

int buffer_front_spare_size(struct buffer *buffer);
#endif
