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

struct arg_lit *help;
struct arg_int *mode, *wordNumber;
struct arg_file *inputFile, *outputFile;
struct arg_str *startingWord;
struct arg_end *end;


int programMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                wchar_t *beginWord) {
    if (setlocale(LC_ALL, "C.UTF-8") == NULL) {
        return -1;
    }
    srand(time(NULL));
    int exitCode = 0;
    h_map *data;
    char *inputPath = strdup(*inputFilePath), *outputPath = strdup(*outputFilePath);

    switch (programMode) {
        case 1:
            data = readFile(inputPath);
            if (data == NULL) {
                exitCode = -1;
                break;
            }
            exitCode = writeCSVFile(data, outputPath);
            freeMap(data);
            break;
        case 2:
            data = readCSVFile(inputPath);
            if (data == NULL) {
                exitCode = -1;
                break;
            }
            exitCode = generateText(data, outputPath, wordN, beginWord);
            break;
    }

    free(inputPath);
    free(outputPath);
    return exitCode;
}

int main(int argc, char **argv) {
    wchar_t *sWord = NULL;
    const char *progname = "csvcraft";
    int exitCode = -1;

    void *argtable[] = {
            help = arg_litn("h", "help", 0, 1, "show program usage"),
            mode = arg_intn("m", "mode", "{0, 1}", 1, 1, "select the task to perform"),
            inputFile = arg_filen("i", "input-file", "<file>", 1, 1, "specify the path of input file"),
            outputFile = arg_filen("o", "output-file", "<file>", 1, 1, "specify the path of output file"),
            wordNumber = arg_intn(NULL, "word-number", "<int>", 0, 1,
                                  "specify the number of words to be generated with task 2"),
            startingWord = arg_strn(NULL, "starting-word", "<string>", 0, 1,
                                    "specify the starting word of the generated text"),
            end = arg_end(10),
    };

    if (arg_nullcheck(argtable) != 0) {
        printf("error: insufficient memory\n");
        exitCode = 0;
        goto exit;
    }

    int nerrors = arg_parse(argc, argv, argtable);
    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s' -h (or --help) for more information.\n", progname);
        exitCode = 0;
        goto exit;
    }

    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "\n");
        exitCode = 0;
        goto exit;
    }

    switch (*mode->ival) {
        case 1:
            if (strcmp(*inputFile->extension, ".txt") != 0 && strcmp(*outputFile->extension, ".csv") != 0) {
                printf("For mode 1 you need to provide a txt input file and a csv one for the output\n");
                exitCode = 0;
                goto exit;
            }
            break;
        case 2:
            if (strcmp(*inputFile->extension, ".csv") != 0 && strcmp(*outputFile->extension, ".txt") != 0) {
                printf("For mode 2 you need to provide a csv input file and a txt one for the output\n");
                exitCode = 0;
                goto exit;
            }
            if (wordNumber->count != 1) {
                printf("Provide a number of word to be generated\n");
                exitCode = 0;
                goto exit;
            }
            if (startingWord->count == 1) {
                sWord = malloc(sizeof(wchar_t) * 40);
                if (sWord == NULL) {
                    printf("out of memory\n");
                    exit(1);
                }
                swprintf(sWord, 40, L"%hs", *(startingWord->sval));
            }
            break;
        default:
            printf("Use:\n1) to select first mode, text -> csv\n2) to select second mode, csv -> text\n");
            exitCode = 0;
            goto exit;
            break;
    }


    exitCode = programMain(*mode->ival, inputFile->filename, outputFile->filename, *wordNumber->ival,
                           sWord);

    exit:
    free(sWord);
    arg_freetable(argtable,
                  sizeof(argtable) / sizeof(argtable[0]));
    return exitCode;
}