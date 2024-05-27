#include "main.h"

// TODO: remove starting point

// lunghezza massima delle parole all'interno del file di input
const int MAX_WORD_LEN = 30;

struct arg_lit *help, *processes;
struct arg_int *mode, *wordNumber;
struct arg_file *inputFile, *outputFile;
struct arg_str *startingWord;
struct arg_end *end;

int singleProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                      wchar_t *beginWord) {
    if (setlocale(LC_ALL, "C.UTF-8") == NULL) {
        perror(NULL);
        return errno;
    }
    srand(time(NULL));
    result_t result;
    h_map *data;
    char *inputPath = strdup(*inputFilePath), *outputPath = strdup(*outputFilePath);

    // eseguo il compito corretto in base al valore di programMode: 1 = compito 1, 2 = compito 2
    result = (programMode == 1) ?
             readAndBuildFileMap(inputPath) :
             readAndBuildCSVMap(inputPath);
    if (result.type != NO_ERROR) {
        // se ci sono stati errori nell'esecuzione di "readAndBuildFileMap" o "readAndBuildCSVMap"
        // printo l'errore e termino il programma
        result.handler(NULL);
        goto exit;
    }
    data = result.success;
    result = (programMode == 1) ?
             buildFileRows(data, 1, outputPath) :
             generateText(data, wordN, beginWord, 1, outputPath);

    if (result.type != NO_ERROR) {
        // se ci sono stati errori nell'esecuzione di "readAndBuildFileMap" o "readAndBuildCSVMap"
        // printo l'errore e termino il programma
        result.handler(NULL);
        freeMap(data);
        goto exit;
    }

    freeMap(data);
    exit:
    free(inputPath);
    free(outputPath);
    return result.type;
}

int multiProcessMain(int programMode, const char **inputFilePath, const char **outputFilePath, int wordN,
                     wchar_t *beginWord) {
    if (setlocale(LC_ALL, "C.UTF-8") == NULL) {
        perror(NULL);
        return errno;
    }
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    srand(time(NULL));
    int fd1[2], fd2[2], pid1, pid2,
            sizeToRead = (programMode == 1) ? sizeof(wchar_t) * (MAX_WORD_LEN + 1)
                                            : sizeof(wchar_t) *
                                              (MAX_WORD_LEN + 10);
    h_map *data;
    char *inputPath = strdup(*inputFilePath), *outputPath = strdup(*outputFilePath);
    // creo le pipe per far comunicare i processi
    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        result.type = errno;
        perror(NULL);
        goto exit;
    }

    // creo il primo processo, sarà il processo che legge il file in input
    pid1 = fork();

    if (pid1 == 0) {
        // child
        // eseguo il compito corretto in base al valore di programMode: 1 = compito 1, 2 = compito 2
        // lettura di file txt o lettura di file csv
        result = (programMode == 1) ? readFile(inputPath, fd1) : readCSVFile(inputPath, fd1);
    } else if (pid1 > 0) {
        // parent
        // creo il secondo processo, sarà il processo che scrive il file di output
        pid2 = fork();

        if (pid2 == 0) {
            // child
            // avvio la funzione di scrittura nel processo figlio
            result = writeFileFromPipe(outputPath, sizeToRead, fd2);
        } else if (pid2 > 0) {
            // parent
            // eseguo il compito corretto in base al valore di programMode: 1 = compito 1, 2 = compito 2
            result = (programMode == 1) ? processFile(fd1) : processCSV(fd1);
            if (result.type != NO_ERROR) {
                // se ci sono stati errori nell'esecuzione di "processFile" o "processCSV"
                // uccido i processi figli, printo l'errore e termino il programma
                kill(pid1, SIGKILL);
                kill(pid2, SIGKILL);
                result.handler(NULL);
                goto exit;
            }
            data = result.success;
            // eseguo il compito corretto in base al valore di programMode: 1 = compito 1, 2 = compito 2
            result = (programMode == 1) ?
                     buildFileRows(data, 0, &fd2) :
                     generateText(data, wordN, beginWord, 0, fd2);

            if (result.type != NO_ERROR) {
                // se ci sono stati errori nell'esecuzione di "processFile" o "processCSV"
                // uccido i processi figli, printo l'errore e termino il programma
                kill(pid1, SIGKILL);
                kill(pid2, SIGKILL);
                result.handler(NULL);
                freeMap(data);
                goto exit;
            }
        } else {
            // si sono verificati errori durante la creazione del secondo processo figlio
            result.type = errno;
            perror(NULL);
            kill(pid1, SIGKILL);
        }
    } else {
        // si sono verificati errori durante la creazione del primo processo figlio
        result.type = errno;
        perror(NULL);
    }

    exit:
    free(inputPath);
    free(outputPath);
    return result.type;
}

int main(int argc, char **argv) {
    // Uso argTable3 per prendere in input le opzioni del programma, fino a riga 174 effettuo controlli sulla validità dell'input.
    wchar_t *sWord = NULL;
    const char *progname = "csvcraft";
    int exitCode = 0;

    // dichiarazione dei parametri presi da linea di comando secondo la documentazione di argTable3
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
        goto exit;
    }

    int nerrors = arg_parse(argc, argv, argtable);
    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s' -h (or --help) for more information.\n", progname);
        goto exit;
    }

    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "\n");
        goto exit;
    }

    if (access(*inputFile->filename, F_OK) != 0 || access(*inputFile->filename, R_OK) != 0) {
        printf("The input file doesn't exists or the program doesn't have read permissions.\n");
        goto exit;
    }

    switch (*mode->ival) {
        case 1:
            // per il compito 1 devo controllare che il file in input sia .txt e quello di output .csv
            if (strcmp(*inputFile->extension, ".txt") != 0 || strcmp(*outputFile->extension, ".csv") != 0) {
                printf("For mode 1 you need to provide a txt input file and a csv one for the output\n");
                goto exit;
            }
            break;
        case 2:
            // per il compito 2 devo controllare che il file in input sia .csv e quello di output .txt
            if (strcmp(*inputFile->extension, ".csv") != 0 || strcmp(*outputFile->extension, ".txt") != 0) {
                printf("For mode 2 you need to provide a csv input file and a txt one for the output\n");
                goto exit;
            }
            // controllo se l'utente ha fornito il parametro obbligatorio "--word-number"
            if (wordNumber->count != 1) {
                printf("Provide a number of word to be generated\n");
                goto exit;
            }
            // controllo se l'utente ha fornito la parola con cui iniziare la generazione
            if (startingWord->count == 1) {
                sWord = malloc(sizeof(wchar_t) * 40);
                if (sWord == NULL) {
                    perror(NULL);
                    exitCode = errno;
                    goto exit;
                }
                swprintf(sWord, 40, L"%hs", *(startingWord->sval));
            }
            break;
        default:
            // l'utente ha scelto di svolgere un compito inesistente
            printf("Use:\n1) to select first mode, text -> csv\n2) to select second mode, csv -> text\n");
            goto exit;
    }


    if (processes->count > 0) {
        // eseguo la task in versione multiprocesso
        exitCode = multiProcessMain(*mode->ival, inputFile->filename, outputFile->filename, *wordNumber->ival,
                                    sWord);
    } else {
        // eseguo la task in versione singolo processo
        exitCode = singleProcessMain(*mode->ival, inputFile->filename, outputFile->filename, *wordNumber->ival,
                                     sWord);
    }

    // pulisco la memoria e ritorno il codice di uscita per terminare il programma
    exit:
    free(sWord);
    arg_freetable(argtable,
                  sizeof(argtable) / sizeof(argtable[0]));
    return exitCode;
}