#include <locale.h>
#include <stdlib.h>
#include <wctype.h>
#include <wchar.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include "include/argtable3.h"
#include "include/fileutil.h"
#include "include/textgen.h"
#include "include/map.h"

int multiProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                wchar_t *beginWord);

int singleProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                wchar_t *beginWord);

int main(int argc, char **argv);