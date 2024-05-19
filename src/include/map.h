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
/*
 * Libera la memoria utilizzata dall'hashmap data in input
*/

void freeMapNodes(h_map *map);
/*
 * Libera la memoria utilizzata dai nodi dell'hashmap data in input
*/

h_node *hNodeBuild(wchar_t *key, void *val);
/*
 * Crea un nodo di tipo h_node e restituisce un puntatore al suo indirizzo di memoria
*/

void hNodeAdd(h_node **head, h_node *node);
/*
 * Aggiunge un nodo di tipo h_node alla linked-list di h_node la cui testa è "h_node **head"
*/

h_map *mapBuild(int cap);
/*
 * Crea un hashmap con capacità "int cap" e restituisce un puntatore al suo indirizzo di memoria
*/

void mapPut(h_map *map, wchar_t *key, void *val);
/*
 * Inserisce all'interno dell'hashmap data in input un nodo di tipo h_node i cui valori sono:
 * node->key = key
 * node->val = val
*/

void *mapGet(h_map *map, wchar_t *key);
/*
 * Recupera dall'hashmap data in input il valore associato al nodo con chiave key e ne restituisce il puntatore.
*/

void mapResize(h_map *map, int newCap);
/*
 * Rialloca la memoria per modificare la capienza dell'hashmap data in input.
 * La nuova dimensione dell'hashmap corrisponde a "int newCap"
*/

#endif
