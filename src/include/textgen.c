#include "textgen.h"

int buildRow(h_node *node, int mode, ...) {
    assert(mode == 0 || mode == 1);
    wchar_t *currWord;
    va_list ptr;
    va_start(ptr, mode);
    int *fd = (mode == 0) ? va_arg(ptr, int *) : NULL, exitCode = NO_ERROR;
    FILE *file = (mode == 1) ? va_arg(ptr, FILE *) : NULL;

    // recupero degli argomenti aggiuntivi e inizializzazione delle variabili in base alla modalità di invocazione
    // 0: single-process, 1: multi-process
    switch (mode) {
        case 0:
            assert(fd != NULL);
            currWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1));
            if (currWord == NULL) {
                exitCode = errno;
                goto exit;
            }
            if (write(fd[1], node->key, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                exitCode = errno;
                free(currWord);
                goto exit;
            }
            break;
        case 1:
            assert(file != NULL);
            if (fwprintf(file, L"%ls", node->key) == -1) {
                exitCode = errno;
                goto exit;
            }
            break;
    }
    va_end(ptr);

    MainNode *mainNode = node->val;
    FreqNode *currFreq;
    h_map *successors = mainNode->successors;
    h_node *currNode;

    // Scorro tutto l'hashmap di successori di "node->key"
    for (int i = 0; i < successors->capacity; i++) {
        currNode = successors->data[i];
        // Per ogni bucket dell'hashmap recupero e analizzo tutti i nodi nel bucket (potrebbero esserci collisioni)
        while (currNode != NULL) {
            // per ogni successore di node->key genero
            // la sua porzione di stringa ossia ",{successore},{frequenza successore}" e la passo alla pipe
            currFreq = currNode->val;
            currFreq->frequency = (double) currFreq->occurrences / mainNode->nSuccessors;

            // in base alla modalità scelta decido dove scrivere il valore calcolato
            switch (mode) {
                case 0:
                    swprintf(currWord, 40, L",%ls,%.4g", currNode->key, currFreq->frequency);
                    if (write(fd[1], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                        exitCode = errno;
                        free(currWord);
                        goto exit;
                    }
                    break;
                case 1:
                    if (fwprintf(file, L",%ls,%.4g", currNode->key, currFreq->frequency) == -1) {
                        exitCode = errno;
                        goto exit;
                    }
                    break;
            }

            free(currFreq);
            currNode = currNode->next;
        }
    }

    // scrivo "\n" sulla "memoria" di output in modo da terminare la riga relativa a node->key e pulisco la memoria
    switch (mode) {
        case 0:
            if (write(fd[1], L"\n", sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                exitCode = errno;
                free(currWord);
                goto exit;
            };
            free(currWord);
            break;
        case 1:
            if (fwprintf(file, L"\n") == -1) {
                exitCode = errno;
                goto exit;
            };
            break;
    }

    freeMap(mainNode->successors);
    free(mainNode);
    exit:
    return exitCode;
}

wchar_t *findNextWord(h_node *node) {
    h_node *curr = node;
    FreqNode *currFreq;

    double choice = (double) rand() / RAND_MAX; // Genero un numero casuale tra [0,1]
    double lowBound = 0, upperBound = 0;

    // L'esecuzione del while termina sempre restituendo una parola.
    while (curr != NULL) {
        currFreq = curr->val;
        lowBound = upperBound;
        upperBound += currFreq->frequency;

        // Se la frequenza della parola corrente è compresa tra lowBound e upperBound oppure il nodo analizzato l'ultimo della lista la scelgo
        // In questo modo, nel caso in cui la somma upperBound non sia precisissima mi assicuro comunque di scegliere una parola.
        if ((lowBound <= choice && choice <= upperBound) || curr->next == NULL) {
            return curr->key;
        }
        curr = curr->next;
    }
    return NULL;
}

wchar_t *findStartingWord(h_map *fileContent) {
    h_node *startingPunctuation;
    int choice;

    // Fino a che non determino "startingPunctuation()" continuo cercare la prima parola.
    // Il ciclo non può essere infinito dato che se fileContent rappresenta il contenuto di un CSV legale questo conterrà sicuramente almeno un "."
    do {
        choice = rand() % 3;
        switch (choice) {
            case 0:
                startingPunctuation = mapGet(fileContent, L".");
                break;
            case 1:
                startingPunctuation = mapGet(fileContent, L"?");
                break;
            case 2:
                startingPunctuation = mapGet(fileContent, L"!");
                break;
        }
    } while (startingPunctuation == NULL);
    // Sceglie casualmente la prima parola tra i successori di startingPunctuation
    return findNextWord(startingPunctuation);
}

result_t generateText(h_map *fileContent, int wordsNumber, wchar_t *startingWord, int mode, ...) {
    assert(mode == 0 || mode == 1); // per gestire gli "errori del programmatore" utilizzo assert
    int error;
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    // Se la parola con cui iniziare il testo è stata fornita controlla se quest'ultima è valida.
    // Se la parola con cui iniziare il testo non è fornita ne sceglie una casualmente.
    if (startingWord == NULL) {
        startingWord = findStartingWord(fileContent);
    } else {
        if (mapGet(fileContent, startingWord) == NULL) {
            result.type = errno = STARTING_WORD_NOT_FOUND;
            result.handler = printErrorMessage;
            goto exit;
        }
    }
    // inizializzo le variabili in base alla modalità scelta e recupero gli argomenti aggiuntivi
    FILE *file = NULL;
    va_list ptr;
    va_start(ptr, mode);
    int *fd = (mode == 0) ? va_arg(ptr, int *) : NULL;
    char *filePath = (mode == 1) ? va_arg(ptr, char *) : NULL;
    va_end(ptr);


    switch (mode) {
        case 0:
            assert(fd != NULL);
            break;
        case 1:
            assert(filePath != NULL);
            file = fopen(filePath, "w");

            if (file == NULL) {
                result.type = errno;
                result.handler = perror;
                goto exit;
            }

            file = freopen(filePath, "a", file);

            if (file == NULL) {
                result.type = errno;
                result.handler = perror;
                goto exit;
            }
            break;
    }

    wchar_t *formattedWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 10));
    if (formattedWord == NULL) {
        result.type = errno;
        result.handler = perror;
        goto exit;
    }
    int capitalizeNext = 1;
    h_node *currFrequencies = mapGet(fileContent, startingWord);
    wchar_t *currWord = startingWord, *nextWord = findNextWord(currFrequencies);

    // Genero le parole casuali fino a che il numero di parole non supera il limite "wordsNumber"
    for (int i = 0; i < wordsNumber; i++) {
        wcscpy(formattedWord, currWord);

        // Sto per scrivere una parola che segue una punteggiatura, voglio che sia "capitalize"
        if (capitalizeNext == 1) {
            capitalize(formattedWord);
            capitalizeNext = 0;
        }

        // Sulla prossima parola andra chiamato "capitalize"
        if (wcscmp(currWord, L"?") == 0 || wcscmp(currWord, L"!") == 0 ||
            wcscmp(currWord, L".") == 0) {
            capitalizeNext = 1;
        }

        // Se la prossima parola non è una punteggiatura allora non aggiungo lo spazio
        // Non voglio: "ciao . come va ?", Voglio: "ciao. Come va?"
        if (wcscmp(nextWord, L"?") != 0 && wcscmp(nextWord, L"!") != 0 &&
            wcscmp(nextWord, L".") != 0) {
            wcscat(formattedWord, L" ");
        }

        // Scrivo nella pipe la parola formattata da scrivere nel TXT di output del compito 2
        // Inizializzo le variabili per il prossimo ciclo
        error = (mode == 0) ?
                write(fd[1], formattedWord, sizeof(wchar_t) * (MAX_WORD_LEN + 10)) :
                fwprintf(file, L"%ls", formattedWord);

        if (error == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit_a;
        }

        currWord = nextWord;
        currFrequencies = mapGet(fileContent, currWord);
        nextWord = findNextWord(currFrequencies);
    }

    // comunico al processo che scrive che ho terminato la generazione delle parole
    if (mode == 0) {
        if (write(fd[1], L"-", sizeof(wchar_t) * 3) == -1) {
            result.type = errno;
            result.handler = perror;
        };
    }

    exit_a:
    free(formattedWord);
    exit:
    purgeCSVNodes(fileContent);
    return result;
}


result_t buildFileRows(h_map *fileContent, int mode, ...) {
    assert(mode == 0 || mode == 1);
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    // Analizzo tutti gli elementi di fileContent scorrendo tutto l'hashmap
    FILE *file;
    va_list ptr;
    va_start(ptr, mode);
    int *fd = (mode == 0) ? va_arg(ptr, int *) : NULL;
    char *filePath = (mode == 1) ? va_arg(ptr, char *) : NULL;
    va_end(ptr);

    switch (mode) {
        case 0:
            assert(fd != NULL);
            break;
        case 1:
            assert(filePath != NULL);
            file = fopen(filePath, "w");

            if (file == NULL) {
                result.type = errno;
                result.handler = perror;
                return result;
            }
            break;
    }

    h_node *currNode;

    for (int i = 0; i < fileContent->capacity; i++) {
        currNode = fileContent->data[i];
        while (currNode != NULL) {
            switch (mode) {
                case 0:
                    result.type = buildRow(currNode, mode, fd);
                    break;
                case 1:
                    result.type = buildRow(currNode, mode, file);
                    break;
            }
            if (result.type != NO_ERROR) {
                result.handler = perror;
                goto exit;
            }
            currNode = currNode->next;
        }
    }

    // comunico al processo che scrive che ho terminato la generazione delle righe
    exit:
    if (mode == 0) {
        if (write(fd[1], L"-", sizeof(wchar_t) * 3) == -1) {
            result.type = errno;
            result.handler = perror;
        };
    }
    return result;
}

result_t processCSV(int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    int newRow = 1;
    h_map *data = mapBuild(0);
    h_node **hHead = malloc(sizeof(h_node *)), *currNode;
    wchar_t *currWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)),
            *headWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)),
            *tmp;
    FreqNode *currFreq;

    if (hHead == NULL || currWord == NULL || headWord == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    // leggo la prima parola dalla pipe
    if (read(fd[0], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
        result.type = errno;
        result.handler = perror;
        goto exit;
    }

    if (wcscmp(currWord, L"-") == 0) {
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        goto exit;
    }

    // Leggo tutte le parole presenti nel CSV tramite la pipe scritta dal processo che legge il CSV
    while (wcscmp(currWord, L"-") != 0) {
        // fine riga csv
        if (wcscmp(currWord, L"\n") == 0) {
            // aggiungo il nodo costruito, in base alla riga appena letta, al dizionario data
            // il valore del nodo inserito nell'hashmap è la testa di una linked-list di h_node
            // ogni h_node rappresenta un successore di headWord
            mapPut(data, headWord, *hHead);
            newRow = 1;
            *hHead = NULL;
            if (read(fd[0], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                result.type = errno;
                result.handler = perror;
                goto exit;
            }
            continue;
        }

        // inizio riga csv
        if (newRow == 1) {
            // prima parola della riga
            wcscpy(headWord, currWord);
            newRow = 0;
        } else {
            // continuo riga csv
            // da ora in poi si alternano "parola,frequenza" fino alla fine della riga ("\n")
            currNode = hNodeBuild(currWord, NULL);
            hNodeAdd(hHead, currNode);
            if (read(fd[0], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                result.type = errno;
                result.handler = perror;
                goto exit;
            }
            currFreq = createFreqNode();
            currFreq->frequency = wcstod(currWord, &tmp);
            currNode->val = currFreq;
        }

        if (read(fd[0], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit;
        }
    }

    result.success = data;
    exit:
    free(hHead);
    free(currWord);
    free(headWord);
    return result;
}


void addToData(h_map *data, wchar_t *prevWord, wchar_t *currWord) {
    // recupero il nodo associato a prevWord all'interno di "data"
    MainNode *curr = mapGet(data, prevWord);
    // provo a recuperare il nodo frequenza associato alla parola currWord
    // all'interno dell'hashmap di successori di curr
    FreqNode *currFreqNode = mapGet(curr->successors, currWord);

    // Se non è presente lo creo
    if (currFreqNode == NULL) {
        currFreqNode = createFreqNode();
        mapPut(curr->successors, currWord, currFreqNode);
    }

    // aggiorno i valori relativi al numero di successori di prevWord e il numero di occorrenze di currWord come
    // successore di prevWord
    curr->nSuccessors++;
    currFreqNode->occurrences++;

    // recupero il nodo associato a currWord in "data"
    curr = mapGet(data, currWord);
    // se non è presente lo creo
    if (curr == NULL) {
        mapPut(data, currWord, createMainNode());
    }
}

result_t processFile(int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    h_map *data = mapBuild(0);
    wchar_t *firstWord, *currWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)), *prevWord = malloc(
            sizeof(wchar_t) * (MAX_WORD_LEN + 1));

    if (currWord == NULL || prevWord == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    // la prima parola è sempre un punto
    wcscpy(prevWord, L".");
    mapPut(data, prevWord, createMainNode());

    // leggo la prima parola reale del file TXT
    if (read(fd[0], currWord, (sizeof(wchar_t) * (MAX_WORD_LEN + 1))) == -1) {
        result.type = errno;
        result.handler = perror;
        goto exit;
    }

    if (wcscmp(currWord, L"-") == 0) {
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        goto exit;
    }

    firstWord = wcsdup(currWord);

    while (wcscmp(currWord, L"-") != 0) {
        // popolo l'hashmap "data"
        addToData(data, prevWord, currWord);
        wcscpy(prevWord, currWord);

        if (read(fd[0], currWord, (sizeof(wchar_t) * (MAX_WORD_LEN + 1))) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit;
        };
    }
    // l'ultima parola del file è seguita dalla prima
    addToData(data, prevWord, firstWord);
    result.success = data;
    exit:
    free(currWord);
    free(prevWord);
    return result;
}

