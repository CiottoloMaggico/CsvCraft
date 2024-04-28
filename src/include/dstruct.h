#ifndef PROGETTO_DSTRUCT_H
#define PROGETTO_DSTRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include "map.h"

typedef struct stringNode {
    struct stringNode *next;
    struct stringNode *previous;
    wchar_t word[];
} StringNode;

typedef struct freqNode {
    double frequency;
    int occurrences;
} FreqNode;

typedef struct mainNode {
    int nSuccessors;
    h_map *successors;
} MainNode;

void capitalize(wchar_t *word);

StringNode *createStrn(wchar_t *word, int format);

void addStrnHead(StringNode **head, StringNode *node);

void addStrnTail(StringNode **tail, StringNode *node);

void printStrn(StringNode **head);

void purgeCSVNodes(h_map *map);

void purgeStringList(StringNode **head, int freeHead);

FreqNode *createFreqNode();

MainNode *createMainNode();



#endif
