#include "map.h"

int MULTIPLY_FACTOR = 2;
int DEFAULT_SIZE = 1024;

void freeMap(h_map *map) {
    freeMapNodes(map);
    free(map);
}

void freeMapNodes(h_map *map) {
    h_node *curr, *prev;

    for (int i = 0; i < map->capacity; i++) {
        curr = map->data[i];
        while (curr != NULL) {
            prev = curr;
            curr = curr->next;
            free(prev->key);
            free(prev);
        }
    }
    free(map->data);
}

static long long hash(h_map *map, wchar_t *data) {
    long long p = 3, result = 0;
    int dataLength = wcslen(data);
    for (int i = 0; i < dataLength; i++) {
        result += ((wint_t) data[i]) * p;
        p *= 3;
    }
    return result % map->capacity;
}

h_node *hNodeBuild(wchar_t *key, void *val) {
    h_node *result = malloc(sizeof(h_node));
    if (result == NULL) {
        printf("out of memory\n");
        exit(-1);
    }
    result->key = wcsdup(key);
    result->val = val;
    result->next = NULL;
    return result;
}

void hNodeAdd(h_node **head, h_node *node) {
    if (*head != NULL) {
        node->next = *head;
    }
    *head = node;
}

h_map *mapBuild(int cap) {
    if (cap <= 2) {
        cap = DEFAULT_SIZE;
    }
    h_map *result = malloc(sizeof(h_map));
    if (result == NULL) {
        printf("out of memory\n");
        exit(-1);
    }

    result->data = calloc(cap, sizeof(h_node));
    if (result->data == NULL) {
        printf("out of memory\n");
        exit(-1);
    }
    result->length = 0;
    result->capacity = cap;
    for (int i = 0; i < cap; i++) {
        result->data[i] = NULL;
    }
    return result;
}

void mapPut(h_map *map, wchar_t *key, void *val) {
    h_node *existing = mapGet(map, key);
    if (existing != NULL) {
        existing->val = val;
        return;
    }
    if (map->length == map->capacity) {
        mapResize(map, 0);
    }
    long long keyHash = hash(map, key);
    h_node *node = malloc(sizeof(h_node));
    if (node == NULL) {
        printf("out of memory\n");
        exit(-1);
    }

    node->next = map->data[keyHash];
    node->key = wcsdup(key);
    node->val = val;
    map->data[keyHash] = node;
    map->length++;
    return;
}

void *mapGet(h_map *map, wchar_t *key) {
    long long keyHash = hash(map, key);
    if (map->data[keyHash] == NULL) {
        return NULL;
    }
    void *result = NULL;
    h_node *curr = map->data[keyHash];

    while (curr != NULL) {
        if (wcscmp(curr->key, key) == 0) {
            result = curr->val;
        }
        curr = curr->next;
    }
    return result;
}

void mapResize(h_map *map, int newCap) {
    if (newCap <= 2 || newCap < map->length) {
        newCap = map->capacity * MULTIPLY_FACTOR;
    }
    h_map *result = mapBuild(newCap);
    h_node *curr;

    for (int i = 0; i < map->capacity; i++) {
        curr = map->data[i];
        while (curr != NULL) {
            mapPut(result, curr->key, curr->val);
            curr = curr->next;
        }
    }

    freeMapNodes(map);
    *map = *result;
    free(result);
}

