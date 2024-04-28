#ifndef PROGETTO_FILEUTIL_H
#define PROGETTO_FILEUTIL_H

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include "textgen.h"
#include "dstruct.h"

h_map *readCSVFile(char *path);

h_map *readFile(char *path);

int writeCSVFile(h_map *fileContent, char *path);

void writeFile(StringNode **head, char *path);

#endif