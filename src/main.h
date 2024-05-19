#include <locale.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include <unistd.h>
#include <time.h>
#include "include/argtable3.h"
#include "include/fileutil.h"
#include "include/textgen.h"
#include "include/map.h"

int programMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                wchar_t *beginWord);

int main(int argc, char **argv);