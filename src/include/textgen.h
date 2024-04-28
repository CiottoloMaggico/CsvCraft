#ifndef PROGETTO_TEXTGEN_H
#define PROGETTO_TEXTGEN_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <float.h>
#include <wctype.h>
#include "dstruct.h"
#include "fileutil.h"
#include "map.h"

wchar_t *findNextWord(h_node *node);

wchar_t *findStartingWord(h_map *head);

int generateText(h_map *fileContent, char *outputPath, int wordsNumber, wchar_t *startingWord);

StringNode **buildRow(h_node *node);

void generateCSVText(h_map *fileContent);

void processCSV(StringNode **head, h_map *data);

void processFile(wchar_t *prevWord, wchar_t *currWord, h_map *data);
#endif