#include "textgen.h"

// utilizzo questi due array per scegliere la prima parola
// con cui iniziare la generazione del testo casuale
// "punctuations" la uso per selezionare una punteggiatura in base ad una potenza di due
static const wchar_t punctuations[][2] = {
        [1] = L"!",
        [2] = L"?",
        [4] = L"."
};

// "scores" la uso per decidere quanto shiftare 1, in base alle punteggiature disponibili nel csv
static const unsigned int scores[][2] = {
        [1] = {0, 0},
        [2] = {1, 1},
        [3] = {0, 1},
        [4] = {2, 2},
        [5] = {0, 2},
        [6] = {1, 2},
};

int buildRow(h_node *node, bool_t singleProcessMode, ...) {
    wchar_t *currWord = NULL;
    va_list ptr;
    va_start(ptr, singleProcessMode);
    int *fd = (singleProcessMode == 0) ? va_arg(ptr, int *) : NULL, exitCode = NO_ERROR;
    FILE *file = (singleProcessMode == 1) ? va_arg(ptr, FILE *) : NULL;

    // recupero degli argomenti aggiuntivi e inizializzazione delle variabili in base alla modalità di invocazione
    // 0: multi-process, 1: single-process
    switch (singleProcessMode) {
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
            switch (singleProcessMode) {
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
    switch (singleProcessMode) {
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

result_t findStartingWord(h_map *fileContent) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };

    // Calcolo lo "score" del file, semplicemente è un numero binario che rappresenta le combinazioni delle
    // punteggiature presenti nel file
    unsigned int score = ((mapGet(fileContent, L".") != NULL) ? pow(2, 2) : 0) +
                         ((mapGet(fileContent, L"?") != NULL) ? pow(2, 1) : 0) +
                         ((mapGet(fileContent, L"!") != NULL) ? pow(2, 0) : 0);


    if (score == 0) {
        // nessuna punteggiatura presente => errore
        result.type = NO_PUNCTUATION;
        result.handler = printErrorMessage;
        return result;
    } else if (score == 7) {
        // posso scegliere tranquillamente tra qualsiasi delle 3
        score = 1 << (rand() % 3);
    } else {
        // calcolo la punteggiatura scelta in base a quanto posso shiftare 1
        // ad esempio:
        // se il file contiene solo "." e "?" => 1 può essere shiftato solo di 0 o 1 posizione
        score = 1 << (scores[score][rand() % 2]);
    }
    result.success = findNextWord(mapGet(fileContent, (wchar_t *) punctuations[score]));
    return result;
}

result_t generateText(h_map *fileContent, int wordsNumber, wchar_t *startingWord, bool_t singleProcessMode, ...) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };

    // Se la parola con cui iniziare il testo è stata fornita controlla se quest'ultima è valida.
    // Se la parola con cui iniziare il testo non è fornita ne sceglie una casualmente.
    if (startingWord == NULL) {
        result = findStartingWord(fileContent);

        if (result.type != NO_ERROR) {
            // Se all'interno del CSV non c'è una punteggiatura comunico all'utente l'errore
            result.type = errno = NO_PUNCTUATION;
            result.handler = printErrorMessage;
            goto exit;
        }
        startingWord = result.success;
    } else {
        if (mapGet(fileContent, startingWord) == NULL) {
            result.type = errno = STARTING_WORD_NOT_FOUND;
            result.handler = printErrorMessage;
            goto exit;
        }
        startingWord = findNextWord(mapGet(fileContent, startingWord));
    }

    FILE *file = NULL;
    va_list ptr;
    va_start(ptr, singleProcessMode);
    int *fd = (singleProcessMode == 0) ? va_arg(ptr, int *) : NULL;
    char *filePath = (singleProcessMode == 1) ? va_arg(ptr, char *) : NULL;
    va_end(ptr);

    switch (singleProcessMode) {
        case 0:
            assert(fd != NULL);
            break;
        case 1:
            assert(filePath != NULL);
            // nel caso in cui il file esiste, lo pulisco e poi inizio a scriverci
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
        if (file != NULL) {fclose(file);}
        goto exit;
    }

    // inizializzo le variabili in base alla modalità scelta e recupero gli argomenti aggiuntivi
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

        // Scrivo nella pipe o nel file la parola formattata da scrivere nel TXT di output del compito 2
        result.type = (singleProcessMode == 0) ?
                      write(fd[1], formattedWord, sizeof(wchar_t) * (MAX_WORD_LEN + 10)) :
                      fwprintf(file, L"%ls", formattedWord);

        if (result.type == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit_a;
        }

        // Inizializzo le variabili per la prossima iterazione
        currWord = nextWord;
        currFrequencies = mapGet(fileContent, currWord);
        nextWord = findNextWord(currFrequencies);
    }

    // comunico al processo che scrive che ho terminato la generazione delle parole
    if (singleProcessMode == 0) {
        if (write(fd[1], L"-", sizeof(wchar_t) * 2) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit_a;
        };
    }

    result.type = NO_ERROR;
    exit_a:
    free(formattedWord);
    if (file != NULL) {
        fclose(file);
    }
    exit:
    purgeCSVNodes(fileContent);
    return result;
}


result_t buildFileRows(h_map *fileContent, bool_t singleProcessMode, ...) {
    // l'argomento singleProcessMode specifica se effettuare il task in versione multi processo o singolo processo
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    // Analizzo tutti gli elementi di fileContent scorrendo tutto l'hashmap
    FILE *file = NULL;
    va_list ptr;
    va_start(ptr, singleProcessMode);
    int *fd = (singleProcessMode == 0) ? va_arg(ptr, int *) : NULL;
    char *filePath = (singleProcessMode == 1) ? va_arg(ptr, char *) : NULL;
    va_end(ptr);

    switch (singleProcessMode) {
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
            switch (singleProcessMode) {
                // per ogni elemento all'interno del hashmap costruisco e scrivo la sua riga per il CSV
                case 0:
                    result.type = buildRow(currNode, singleProcessMode, fd);
                    break;
                case 1:
                    result.type = buildRow(currNode, singleProcessMode, file);
                    break;
            }
            if (result.type != NO_ERROR) {
                result.handler = perror;
                goto exit;
            }
            currNode = currNode->next;
        }
    }

    exit:
    if (singleProcessMode == 0) {
        // comunico al processo che scrive che ho terminato la generazione delle righe
        if (write(fd[1], L"-", sizeof(wchar_t) * 2) == -1) {
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
            // aggiungo il nodo costruito, in base alla riga appena letta, al dizionario "data"
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
            // quindi leggo la coppia e poi passo alla successiva
            currNode = hNodeBuild(currWord, NULL); // costruisco il nodo
            hNodeAdd(hHead, currNode); // lo aggiungo alla lista dei successori di headWord
            // leggo la frequenza relativa a "currWord"
            if (read(fd[0], currWord, sizeof(wchar_t) * (MAX_WORD_LEN + 1)) == -1) {
                result.type = errno;
                result.handler = perror;
                goto exit;
            }
            currFreq = createFreqNode(); // creo il nodo frequenza
            currFreq->frequency = wcstod(currWord,
                                         &tmp); // salvo la frequenza del successore all'interno del nodo frequenza
            currNode->val = currFreq; // collego il nodo frequenza a "currNode"
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


MainNode *addToData(h_map *data, wchar_t **firstWord, MainNode *currWord, wchar_t *nextWord) {
    // Se currWord == NULL => nextWord è la prima parola del file
    if (currWord != NULL) {
        // provo a recuperare il nodo frequenza associato alla parola nextWord
        // all'interno dell'hashmap di successori di currWord
        FreqNode *currFreqNode = mapGet(currWord->successors, nextWord);

        if (currFreqNode == NULL) {
            // nextWord è un nuovo successore quindi lo aggiungo ai successori di currWord
            currFreqNode = createFreqNode();
            mapPut(currWord->successors, nextWord, currFreqNode);
        }

        // aggiorno i valori relativi al numero di successori di currWord e
        // il numero di occorrenze di nextWord come successore di currWord
        currWord->nSuccessors++;
        currFreqNode->occurrences++;
    }

    // provo a recuperare il nodo associato a nextWord in "data"
    MainNode *curr = mapGet(data, nextWord);
    if (curr == NULL) {
        // Non esiste un nodo con chiave nextWord quindi lo aggiungo all'hashmap delle parole
        h_node *addedNode = mapPut(data, nextWord, createMainNode());

        if (currWord == NULL) {
            // "nextWord" è la prima parola del file txt in input
            *firstWord = addedNode->key;
        }
        return addedNode->val;
    }
    return curr;
}

result_t processFile(int fd[]) {
    result_t result = {
            .type = NO_ERROR,
            .success = NULL,
            .handler = NULL,
    };
    h_map *data = mapBuild(0);
    MainNode *currWord = NULL;
    wchar_t *nextWord = malloc(sizeof(wchar_t) * (MAX_WORD_LEN + 1)),
            **firstWord = malloc(sizeof(wchar_t *));

    if (nextWord == NULL || firstWord == NULL) {
        result.type = errno;
        result.handler = perror;
        return result;
    }

    // leggo la prima parola reale del file TXT
    if (read(fd[0], nextWord, (sizeof(wchar_t) * (MAX_WORD_LEN + 1))) == -1) {
        result.type = errno;
        result.handler = perror;
        goto exit;
    }

    if (wcscmp(nextWord, L"-") == 0) {
        // il file txt è vuoto, comunico l'errore al chiamante
        result.type = errno = EMPTY_FILE;
        result.handler = printErrorMessage;
        goto exit;
    }

    while (wcscmp(nextWord, L"-") != 0) {
        // popolo l'hashmap "data" finchè non ricevo il carattere che indica che il file è finito
        currWord = addToData(data, firstWord, currWord, nextWord);

        if (read(fd[0], nextWord, (sizeof(wchar_t) * (MAX_WORD_LEN + 1))) == -1) {
            result.type = errno;
            result.handler = perror;
            goto exit;
        };
    }

    // l'ultima parola del file è seguita dalla prima
    addToData(data, firstWord, currWord, *firstWord);
    result.success = data;
    exit:
    free(nextWord);
    free(firstWord);
    return result;
}

