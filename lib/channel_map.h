#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H

#include "channel.h"

/*
 *channel 映射表, key为对应的socket描述字
 *
 */

struct channel_map {
    void **entries;
    int nentries;
};

int map_make_space(struct channel_map *map, int slot, int msize);
void map_init(struct channel_map *map);
void map_clear(struct channel_map *map);

#endif
