#include "fileutil.h"

wchar_t *nextCSVColumn(FILE *file) {
    wchar_t *result = malloc(sizeof(wchar_t) * 31), *charToString = malloc(sizeof(wchar_t) * 2);
    wint_t currChar = fgetwc(file);

    switch (currChar) {
        case WEOF:
            free(result);
            result = NULL;
            break;
        case '\n':
            swprintf(result, 2, L"%lc", currChar);
            break;
        default:
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

h_map *readAndBuildFileMap(char *path) {
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2), *firstWord = malloc(sizeof(wchar_t) * 35), *prevWord = malloc(
            sizeof(wchar_t) * 35), *currWord = malloc(
            sizeof(wchar_t) * 35);
    if (currWord == NULL || prevWord == NULL || firstWord == NULL || charToString == NULL) {
        printf("out of memory\n");
        return NULL;
    }

    wcscpy(prevWord, L".");
    h_map *data = mapBuild(0);
    mapPut(data, prevWord, createMainNode());
    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    if (currChar == WEOF) {
        printf("Il file di testo è vuoto!\n");
        fclose(file);
        free(firstWord);
        free(prevWord);
        free(currWord);
        free(charToString);
        return NULL;
    }

    while (currChar != WEOF) {
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (iswalnum(currChar) == 0) {
            if (data->length == 1) {
                wcscpy(firstWord, currWord);
            }
            if (wordLen != 0) {
                if (currChar == '\'') {
                    wcscat(currWord, charToString);
                    wordLen++;
                }
                addToData(data, prevWord, currWord);
                wcscpy(prevWord, currWord);
            }
            if (currChar == '!' || currChar == '?' || currChar == '.') {
                wcscpy(currWord, charToString);
                addToData(data, prevWord, currWord);
                wcscpy(prevWord, currWord);
            }
            wordLen = 0;
        } else {
            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }
    if (wordLen != 0) {
        addToData(data, prevWord, currWord);
    }
    addToData(data, currWord, firstWord);

    fclose(file);
    free(firstWord);
    free(prevWord);
    free(currWord);
    free(charToString);
    return data;
}

h_map *readAndBuildCSVMap(char *path) {
    h_map *data = mapBuild(0);
    wchar_t *tmp, *mainWord = NULL, *currWord;
    h_node **hHead = malloc(sizeof(h_node *)), *currNode;

    if (hHead == NULL) {
        printf("out of memory\n");
        return NULL;
    }

    int mode = 0;
    FILE *file = fopen(path, "r");
    FreqNode *currFreq;
    currWord = nextCSVColumn(file);

    if (currWord == NULL) {
        printf("Il file di testo è vuoto!\n");
        free(hHead);
        fclose(file);
        return NULL;
    }

    while (currWord != NULL) {
        // fine riga csv
        if (wcscmp(currWord, L"\n") == 0) {
            // aggiungo il nodo costruito in base alla riga appena letta al dizionario data
            // il valore del nodo inserito nell'hashmap è la testa di una linked-list di h_node
            // ogni h_node rappresenta un successore di mainWord
            mapPut(data, mainWord, *hHead);
            free(mainWord);
            mainWord = NULL;
            *hHead = NULL;
            currWord = nextCSVColumn(file);
            continue;
        }

        // inizio riga csv
        if (mainWord == NULL) {
            // prima parola della riga
            mainWord = wcsdup(currWord);
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
        currWord = nextCSVColumn(file);
    }

    fclose(file);
    return data;
}

int readCSVFile(char *path, int fd[]) {
    FILE *file = fopen(path, "r");
    wchar_t *currWord = nextCSVColumn(file);

    while (currWord != NULL) {
        write(fd[1], currWord, sizeof(wchar_t) * 31);
        free(currWord);
        currWord = nextCSVColumn(file);
    }

    write(fd[1], L"--", sizeof(wchar_t) * 3);
    fclose(file);
    return 1;
}

void readFile(char *path, int fd[]) {
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2), *currWord = malloc(sizeof(wchar_t) * 35);
    if (currWord == NULL || charToString == NULL) {
        printf("out of memory\n");
        return;
    }

    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    while (currChar != WEOF) {
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (iswalnum(currChar) == 0) {
            if (wordLen != 0) {
                if (currChar == '\'') {
                    wcscat(currWord, charToString);
                    wordLen++;
                }
                write(fd[1], currWord, sizeof(wchar_t) * 35);
            }
            if (currChar == '!' || currChar == '?' || currChar == '.') {
                wcscpy(currWord, charToString);
                write(fd[1], currWord, sizeof(wchar_t) * 35);
            }
            wordLen = 0;
        } else {
            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }
    if (wordLen != 0) {
        write(fd[1], currWord, sizeof(wchar_t) * 35);
    }
    write(fd[1], L"--", sizeof(wchar_t) * 3);
    fclose(file);
    free(currWord);
    free(charToString);
    return;
}


int writeFileFromPipe(char *path, int fd[]) {
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return -2;
    }

    file = freopen(path, "a", file);

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return -2;
    }

    wchar_t *string = malloc(sizeof(wchar_t) * 40);

    if (read(fd[0], string, (sizeof(wchar_t) * 40)) == -1) {
        printf("errore");
    }

    while (wcscmp(string, L"--") != 0) {
        fwprintf(file, L"%ls", string);
        if (read(fd[0], string, (sizeof(wchar_t) * 40)) == -1) {
            printf("errore");
        }
    }

    fclose(file);
    free(string);
    return 0;
}

