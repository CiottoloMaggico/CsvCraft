#ifndef PROGETTO_MAP_H
#define PROGETTO_MAP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

extern int MULTIPLY_FACTOR;
extern int DEFAULT_SIZE;

typedef struct hNode {
    wchar_t *key;
    void *val;
    struct hNode *next;
} h_node;

typedef struct hMap{
    int length;
    int capacity;
    struct hNode **data;
} h_map;

void freeMap(h_map *map);

void freeMapNodes(h_map *map);

h_node *hNodeBuild(wchar_t *key, void *val);

void hNodeAdd(h_node **head, h_node *node);

h_map *mapBuild(int cap);

void mapPut(h_map *map, wchar_t *key, void *val);

void *mapGet(h_map *map, wchar_t *key);

void mapResize(h_map *map, int newCap);

#endif
