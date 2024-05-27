#include "map.h"

// Ogni volta che l'hashmap si riempie e viene ricreata la sua capacità sarà il doppio di quella precedente
int MULTIPLY_FACTOR = 2;
// Grandezza di default dell'hashmap creata se non viene specificata una grandezza valida
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


// Calcola l'hash per il valora in data in base all'hashmap "map" data in input
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
        perror("fatal error, crashing");
        exit(errno);
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
        // grandezza invalida, la capienza è DEFAULT_SIZE
        cap = DEFAULT_SIZE;
    }
    h_map *result = malloc(sizeof(h_map));
    if (result == NULL) {
        perror("fatal error, crashing");
        exit(errno);
    }

    // alloco la memoria per l'array di "cap" bucket
    result->data = calloc(cap, sizeof(h_node));
    if (result->data == NULL) {
        perror("fatal error, crashing");
        exit(errno);
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
        // L'hashmap è piena, deve essere ricreata con una capacità maggiore
        mapResize(map, 0);
    }
    // calcolo l'hash della chiave dell'elemento da inserire
    long long keyHash = hash(map, key);
    h_node *node = malloc(sizeof(h_node));
    if (node == NULL) {
        perror("fatal error, crashing");
        exit(errno);
    }

    // lo inserisco gestendo le eventuali collisioni con una linked-list
    node->next = map->data[keyHash];
    node->key = wcsdup(key);
    node->val = val;
    map->data[keyHash] = node;
    map->length++;
    return;
}

void *mapGet(h_map *map, wchar_t *key) {
    // calcolo l'hash della chiave fornita
    long long keyHash = hash(map, key);
    if (map->data[keyHash] == NULL) {
        // il bucket relativo all'hash è vuoto
        return NULL;
    }
    void *result = NULL;
    h_node *curr = map->data[keyHash];

    while (curr != NULL) {
        if (wcscmp(curr->key, key) == 0) {
            // ho trovato il valore relativo alla chiave in input
            result = curr->val;
            return result;
        }
        curr = curr->next;
    }
    // l'elemento cercato non è nell'hashmap
    return result;
}

void mapResize(h_map *map, int newCap) {
    // controllo se "newCap" ha un valore valido
    if (newCap <= 2 || newCap < map->length) {
        // "newCap" non ha un valore valido quindi ingrandisco l'hashmap di "map->capacity * MULTIPLY_FACTOR"
        newCap = map->capacity * MULTIPLY_FACTOR;
    }
    h_map *result = mapBuild(newCap);
    h_node *curr;

    // copio gli elementi della vecchia hashmap nella nuova
    for (int i = 0; i < map->capacity; i++) {
        curr = map->data[i];
        while (curr != NULL) {
            mapPut(result, curr->key, curr->val);
            curr = curr->next;
        }
    }

    freeMapNodes(map);
    // scambio i puntatori all'hashmap in modo che il puntatore map punti alla nuova hashmap
    *map = *result;
    free(result);
}

