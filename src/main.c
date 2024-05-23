#include "main.h"

struct arg_lit *help, *processes;
struct arg_int *mode, *wordNumber;
struct arg_file *inputFile, *outputFile;
struct arg_str *startingWord;
struct arg_end *end;

int singleProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
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
            data = readAndBuildFileMap(inputPath);
            if (data == NULL) {
                exitCode = 5;
                break;
            }
            buildFileRows(data, 1, outputPath);
            break;
        case 2:
            data = readAndBuildCSVMap(inputPath);
            if (data == NULL) {
                exitCode = 5;
                break;
            }
            exitCode = generateText(data, wordN, beginWord, 1, outputPath);
            break;
    }
    return exitCode;
}


int multiProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                     wchar_t *beginWord) {
    // eseguo il compito corretto in base alle opzioni prese in input
    if (setlocale(LC_ALL, "C.UTF-8") == NULL) {
        return -1;
    }
    srand(time(NULL));
    int exitCode = 0, fd1[2], fd2[2], pid;
    h_map *data;
    char *inputPath = strdup(*inputFilePath), *outputPath = strdup(*outputFilePath);


    switch (programMode) {
        case 1:
            pipe(fd1);
            pipe(fd2);
            pid = fork();

            if (pid == 0) {
                close(fd1[0]);
                readFile(inputPath, fd1);
                close(fd1[1]);
            } else if (pid > 0) {
                pid = fork();

                if (pid == 0) {
                    close(fd2[1]);
                    exitCode = writeFileFromPipe(outputPath, fd2);

                    close(fd2[0]);
                } else if (pid > 0) {
                    close(fd1[1]);
                    close(fd2[0]);
                    h_map *dataHashMap = processFile(fd1);
                    if (dataHashMap == NULL) {
                        kill(pid, SIGKILL);
                        break;
                    }
                    close(fd1[0]);
                    buildFileRows(dataHashMap, 0, &fd2);
                    close(fd2[1]);
                    freeMap(dataHashMap);
                } else if (pid == -1) {
                    exit(-1);
                }
            } else if (pid == -1) {
                exit(-1);
            }
            break;
        case 2:
            pipe(fd1);
            pipe(fd2);
            pid = fork();

            if (pid == 0) {
                close(fd1[0]);
                readCSVFile(inputPath, fd1);
                close(fd1[1]);
            } else if (pid > 0) {
                pid = fork();

                if (pid == 0) {
                    exitCode = writeFileFromPipe(outputPath, fd2);
                } else if (pid > 0) {
                    data = processCSV(fd1);
                    if (data == NULL) {
                        kill(pid, SIGKILL);
                        break;
                    }
                    exitCode = generateText(data, wordN, beginWord, 0, fd2);
                } else if (pid == -1) {
                    exit(-1);
                }
            } else if (pid == -1) {
                exit(-1);
            }
            break;
    }

    _exit(exitCode);

    free(inputPath);
    free(outputPath);
    return exitCode;
}

int main(int argc, char **argv) {
    // Uso TableArg3 per prendere in input le opzioni del programma, fino a riga 174 effettuo controlli sulla validità dell'input.
    wchar_t *sWord = NULL;
    const char *progname = "csvcraft";
    int exitCode = -1;

    void *argtable[] = {
            help = arg_litn("h", "help", 0, 1, "show program usage"),
            mode = arg_intn("m", "mode", "{1, 2}", 1, 1, "select the task to perform"),
            processes = arg_litn("p", "multi-process", 0, 1, "execute the task in multiprocess mode"),
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

    if (access(*inputFile->filename, F_OK) != 0 || access(*inputFile->filename, R_OK) != 0) {
        printf("The input file doesn't exists or the program doesn't have read permissions.\n");
        exitCode = 0;
        goto exit;
    }

    switch (*mode->ival) {
        case 1:
            if (strcmp(*inputFile->extension, ".txt") != 0 || strcmp(*outputFile->extension, ".csv") != 0) {
                printf("For mode 1 you need to provide a txt input file and a csv one for the output\n");
                exitCode = 0;
                goto exit;
            }
            break;
        case 2:
            if (strcmp(*inputFile->extension, ".csv") != 0 || strcmp(*outputFile->extension, ".txt") != 0) {
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


    if (processes->count > 0) {
        exitCode = multiProcessMain(*mode->ival, inputFile->filename, outputFile->filename, *wordNumber->ival,
                                    sWord);
    } else {
        exitCode = singleProcessMain(*mode->ival, inputFile->filename, outputFile->filename, *wordNumber->ival,
                                     sWord);
    }

    exit:
    free(sWord);
    arg_freetable(argtable,
                  sizeof(argtable) / sizeof(argtable[0]));
    return exitCode;
}