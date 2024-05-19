#include "textgen.h"

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

int generateText(h_map *fileContent, int wordsNumber, wchar_t *startingWord, int fd[]) {
    // Se la parola con cui iniziare il testo è stata fornita controlla se quest'ultima è valida.
    // Se la parola con cui iniziare il testo non è fornita ne sceglie una casualmente.
    if (startingWord == NULL) {
        startingWord = findStartingWord(fileContent);
    } else {
        if (mapGet(fileContent, startingWord) == NULL) {
            printf("Starting word not found.\n");
            return -1;
        }
    }

    wchar_t *formattedWord = malloc(sizeof(wchar_t) * 40);
    if (formattedWord == NULL) {
        printf("out of memory\n");
        return -1;
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
        write(fd[1], formattedWord, sizeof(wchar_t) * 40);
        currWord = nextWord;
        currFrequencies = mapGet(fileContent, currWord);
        nextWord = findNextWord(currFrequencies);
    }

    write(fd[1], L"--", sizeof(wchar_t) * 3);
    purgeCSVNodes(fileContent);
    freeMap(fileContent);
    free(formattedWord);
    return 0;
}

void buildRow(h_node *node, int fd[]) {
    wchar_t *currWord = malloc(sizeof(wchar_t) * 40);
    if (currWord == NULL) {
        printf("Out of memory\n");
        exit(-1);
    }
    MainNode *mainNode = node->val;
    FreqNode *currFreq;
    h_map *successors = mainNode->successors;
    h_node *currNode;

    // Scrivo prima parola della riga del CSV nella pipe
    write(fd[1], node->key, sizeof(wchar_t) * 40);

    // Scorro tutto l'hashmap di successori di "node->key"
    for (int i = 0; i < successors->capacity; i++) {
        currNode = successors->data[i];
        // Per ogni bucket dell'hashmap recupero e analizzo tutti i nodi nel bucket (potrebbero esserci collisioni)
        while (currNode != NULL) {
            // per ogni successore di node->key genero
            // la sua porzione di stringa ossia ",{successore},{frequenza successore}" e la passo alla pipe
            currFreq = currNode->val;
            currFreq->frequency = (double) currFreq->occurrences / mainNode->nSuccessors;
            swprintf(currWord, 40, L",%ls,%.4g", currNode->key, currFreq->frequency);
            write(fd[1], currWord, sizeof(wchar_t) * 40);
            free(currFreq);
            currNode = currNode->next;
        }
    }

    // scrivo "\n" sulla pipe in modo da terminare la riga relativa a node->key e pulisco la memoria
    write(fd[1], L"\n", sizeof(wchar_t) * 40);
    freeMap(mainNode->successors);
    free(mainNode);
    free(currWord);
    return;
}

void buildFileRows(h_map *fileContent, int fd[]) {
    // Analizzo tutti gli elementi di fileContent scorrendo tutto l'hashmap
    h_node *currNode;

    for (int i = 0; i < fileContent->capacity; i++) {
        currNode = fileContent->data[i];
        while (currNode != NULL) {
            buildRow(currNode, fd);
            currNode = currNode->next;
        }
    }

    write(fd[1], L"--", sizeof(wchar_t) * 3);
    return;
}

h_map *processCSV(int fd[]) {
    h_map *data = mapBuild(0);
    h_node **hHead = malloc(sizeof(h_node *)), *currNode;
    wchar_t *currWord = malloc(sizeof(wchar_t) * 31), *headWord, *tmp;
    FreqNode *currFreq;
    headWord = NULL;
    int mode = 0;

    if (hHead == NULL) {
        printf("Out of memory.\n");
        exit(-1);
    }

    // Leggo tutte le parole presenti nel CSV tramite la pipe scritta dal processo che legge il CSV
    while (1) {
        // Lettura nella pipe
        if (read(fd[0], currWord, sizeof(wchar_t) * 31) == -1) {
            printf("error");
            _exit(-1);
        }

        // file CSV terminato, tutte le parole sono state lette
        if (wcscmp(currWord, L"--") == 0) {
            break;
        }

        // fine riga csv
        if (wcscmp(currWord, L"\n") == 0) {
            // aggiungo il nodo costruito in base alla riga appena letta al dizionario data
            // il valore del nodo inserito nell'hashmap è la testa di una linked-list di h_node
            // ogni h_node rappresenta un successore di headWord
            mapPut(data, headWord, *hHead);
            free(headWord);
            headWord = NULL;
            *hHead = NULL;
            continue;
        }

        // inizio riga csv
        if (headWord == NULL) {
            // prima parola della riga
            headWord = wcsdup(currWord);
            continue;
        } else {
            // continuo riga csv
            // da ora in poi si alternano "parola,frequenza" fino alla fine della riga ("\n")
            switch (mode) {
                case 0: // parola
                    currNode = hNodeBuild(currWord, NULL);
                    hNodeAdd(hHead, currNode);
                    mode = 1;
                    break;
                case 1: // frequenza
                    currFreq = createFreqNode();
                    currFreq->frequency = wcstod(currWord, &tmp);
                    currNode->val = currFreq;
                    mode = 0;
                    break;
            }
        }
    }
    free(hHead);
    free(currWord);
    return data;
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

h_map *processFile(int fd[]) {
    h_map *data = mapBuild(0);
    wchar_t *firstWord, *currWord = malloc(sizeof(wchar_t) * 35), *prevWord = malloc(sizeof(wchar_t) * 35);

    if (currWord == NULL || prevWord == NULL) {
        printf("Out of memory.\n");
        exit(-1);
    }

    // la prima parola è sempre un punto
    wcscpy(prevWord, L".");
    mapPut(data, prevWord, createMainNode());

    // leggo la prima parola reale del file TXT
    if (read(fd[0], currWord, (sizeof(wchar_t) * 35)) == -1) {
        printf("errore\n");
    };

    if (wcscmp(currWord, L"--") == 0) {
        printf("Il file di testo è vuoto!\n");
        free(currWord);
        free(prevWord);
        return NULL;
    }

    firstWord = wcsdup(currWord);

    while (1) {
        // popolo l'hashmap "data"
        addToData(data, prevWord, currWord);
        wcscpy(prevWord, currWord);
        if (read(fd[0], currWord, (sizeof(wchar_t) * 35)) == -1) {
            printf("errore\n");
        };
        if (wcscmp(currWord, L"--") == 0) {
            // la parola successiva all'ultima parola del file è la prima
            addToData(data, prevWord, firstWord);
            break;
        }
    }

    free(currWord);
    free(prevWord);
    return data;
}

