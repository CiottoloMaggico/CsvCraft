#include "fileutil.h"

wchar_t *nextCSVColumn(FILE *file) {
    wchar_t *result = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)),
            *charToString = malloc(sizeof(wchar_t) * 2);

    if (result == NULL || charToString == NULL) {
        perror(NULL);
        return NULL;
    }

    wint_t currChar = fgetwc(file);

    switch (currChar) {
        case WEOF:
            // primo carattere "WEOF"
            swprintf(result, 2, L"-");
            break;
        case '\n':
            // primo carattere "\n"
            swprintf(result, 2, L"%lc", currChar);
            break;
        default:
            // qualsiasi altro carattere, indica l'inizio di una nuova parola/colonna
            wcscpy(result, L"");

            while (currChar != ',' && currChar != '\n' && currChar != WEOF) {
                swprintf(charToString, 2, L"%lc", currChar);
                wcscat(result, charToString);
                currChar = fgetwc(file);
            }

            if (currChar != ',') {
                ungetwc(currChar, file);
            }
            break;
    }

    free(charToString);
    return result;
}

result_t readAndBuildFileMap(char *path) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2),
            *nextWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)),
            **firstWord = malloc(sizeof(wchar_t *));

    if (nextWord == NULL || charToString == NULL || firstWord == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    // inizializzo la struttura dati che conterrà il contenuto del file
    h_map *data = mapBuild(0);
    MainNode *currWord = NULL;
    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    if (currChar == WEOF) {
        // il file è vuoto, ritorno il codice di errore
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        goto exit;
    }

    while (currChar != WEOF) {
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (iswalnum(currChar) == 0) {
            // currChar è un "separatore" di parola
            if (wordLen != 0) {
                // La lunghezza della parola è maggiore di zero quindi la salvo
                if (currChar == '\'') {
                    // l'apice viene considerato parte della parola ma carattere di separazione,
                    // quindi lo concateno a nextWord
                    wcscat(nextWord, charToString);
                    wordLen++;
                }
                currWord = addToData(data, firstWord, currWord, nextWord);
            }
            if (currChar == '!' || currChar == '?' || currChar == '.') {
                // caratteri di separazione speciali che vengono considerati parole a se
                wcscpy(nextWord, charToString);
                currWord = addToData(data, firstWord, currWord, nextWord);
            }
            wordLen = 0;
        } else {
            if (wordLen == 0) {
                // primo carattere di una nuova parola, quindi "resetto" nextWord
                wcscpy(nextWord, charToString);
            } else {
                // caratteri successivi al primo, li concateno a nextWord
                wcscat(nextWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }
    if (wordLen != 0) {
        // il testo è terminato senza un carattere di separazione quindi aggiungo la parola finale del file
        // alla struttura dati
        currWord = addToData(data, firstWord, currWord, nextWord);
    }
    // l'ultima parola si intende seguita dalla prima
    addToData(data, firstWord, currWord, *firstWord);
    result.success = data;

    exit:
    fclose(file);
    free(firstWord);
    free(nextWord);
    free(charToString);
    return result;
}

result_t readAndBuildCSVMap(char *path) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    int newRow = 1;
    h_map *data = mapBuild(0);
    wchar_t *tmp, *mainWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)), *currWord;
    h_node **hHead = malloc(sizeof(h_node *)), *currNode;

    if (hHead == NULL || mainWord == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        result.type = errno;
        result.handler = perror;
        free(hHead);
        goto exit;
    }

    FreqNode *currFreq;
    // leggo la prima parola del CSV
    currWord = nextCSVColumn(file);

    if (wcscmp(currWord, L"-") == 0) {
        // il file è vuoto, ritorno l'opportuno codice di errore
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        free(hHead);
        goto exit;
    }

    while (wcscmp(currWord, L"-") != 0) {
        // fine riga csv
        if (*currWord == '\n') {
            // aggiungo il nodo costruito in base alla riga appena letta al dizionario "data".
            // Il valore del nodo inserito nell'hashmap è la testa di una linked-list di h_node
            // ogni h_node rappresenta un successore di mainWord.
            mapPut(data, mainWord, *hHead);
            newRow = 1;
            *hHead = NULL;
            currWord = nextCSVColumn(file);
            continue;
        }

        // inizio riga csv
        if (newRow == 1) {
            // prima parola della riga
            wcscpy(mainWord, currWord);
            newRow = 0;
        } else {
            // continuo riga csv
            // da ora in poi si alternano "parola,frequenza" fino alla fine della riga ("\n")
            // costruisco il nodo che rappresenta la parola
            currNode = hNodeBuild(currWord, NULL);
            hNodeAdd(hHead, currNode);
            // leggo la frequenza relativa a "currWord"
            currWord = nextCSVColumn(file);
            // costruisco il "nodo" che conterrà le informazioni riguardo alla frequenza della parola
            currFreq = createFreqNode();
            currFreq->frequency = wcstod(currWord, &tmp);
            currNode->val = currFreq;
        }
        currWord = nextCSVColumn(file);
    }

    result.success = data;
    exit:
    fclose(file);
    free(mainWord);
    return result;
}

result_t readCSVFile(char *path, int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }
    wchar_t *currWord = nextCSVColumn(file);

    while (wcscmp(currWord, L"-") != 0) {
        // leggo le colonne del csv finchè non finisce il file e le scrivo in una pipe
        if (write(fd[1], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit;
        };
        free(currWord);
        currWord = nextCSVColumn(file);
    }

    if (write(fd[1], L"-", sizeof(wchar_t) * 2) == -1) {
        // comunico al processo che legge che ho finito il file
        result.type = errno;
        result.handler = perror;
    };

    exit:
    fclose(file);
    return result;
}

result_t readFile(char *path, int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2),
            *currWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1));
    if (currWord == NULL || charToString == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        result.type = errno;
        result.handler = perror;
        goto exit_b;
    }
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    while (currChar != WEOF) {
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (iswalnum(currChar) == 0) {
            // currChar è un "separatore" di parola
            if (wordLen != 0) {
                // La lunghezza della parola è maggiore di zero quindi la salvo
                if (currChar == '\'') {
                    // l'apice viene considerato parte della parola ma carattere di separazione,
                    // quindi lo concateno a currWord
                    wcscat(currWord, charToString);
                    wordLen++;
                }
                // scrivo la parola appena trovata in una pipe letta dal processo che crea la struttura dati
                if (write(fd[1], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                    result.type = errno;
                    result.handler = perror;
                    goto exit;
                };
            }
            if (currChar == '!' || currChar == '?' || currChar == '.') {
                // caratteri di separazione speciali che vengono considerati parole a se
                wcscpy(currWord, charToString);
                // scrivo la parola appena trovata in una pipe letta dal processo che crea la struttura dati
                if (write(fd[1], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                    result.type = errno;
                    result.handler = perror;
                    goto exit;
                };
            }
            wordLen = 0;
        } else {
            if (wordLen == 0) {
                // primo carattere di una nuova parola, quindi "resetto" currWord
                wcscpy(currWord, charToString);
            } else {
                // caratteri successivi al primo, li concateno a currWord
                wcscat(currWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }
    if (wordLen != 0) {
        // il testo è terminato senza un carattere di separazione quindi scrivo nella pipe la parola finale del file
        if (write(fd[1], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit;
        };
    }

    exit:
    fclose(file);
    exit_b:
    // comunico al processo che legge che ho terminato di leggere il file
    if (write(fd[1], L"-", sizeof(wchar_t) * 2) == -1) {
        result.type = errno;
        result.handler = perror;
    };
    free(currWord);
    free(charToString);
    return result;
}


result_t writeFileFromPipe(char *path, int sizeToRead, int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };

    wchar_t *string = malloc(sizeToRead);
    if (string == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    if (read(fd[0], string, sizeToRead) == -1) {
        result.type = errno;
        result.handler = perror;
        goto exit;
    }

    if (wcscmp(string, L"-") == 0) {
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        goto exit;
    }

    FILE *file = fopen(path, "w");

    if (file == NULL) {
        result.type = errno;
        result.handler = perror;
        goto exit_a;
    }

    file = freopen(path, "a", file);

    if (file == NULL) {
        result.type = errno;
        result.handler = perror;
        goto exit_a;
    }

    while (wcscmp(string, L"-") != 0) {
        // semplicemente leggo dalla pipe delle parole e le scrivo sul file di output
        if (fwprintf(file, L"%ls", string) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit_a;
        };
        if (read(fd[0], string, sizeToRead) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit_a;
        }
    }

    exit_a:
    fclose(file);
    exit:
    free(string);
    return result;
}

