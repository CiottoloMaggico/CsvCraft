#include "fileutil.h"


h_map *readCSVFile(char *path) {
    h_map *data = mapBuild(0);
    StringNode *curr, **head = malloc(sizeof(StringNode *)), **tail = malloc(sizeof(StringNode *));
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2), *currWord = malloc(sizeof(wchar_t) * 31);

    if (head == NULL || tail == NULL || charToString == NULL || currWord == NULL) {
        printf("out of memory\n");
        return NULL;
    }

    int wordLen = 0;
    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    *head = *tail = NULL;

    while (currChar != WEOF) {
        if (currChar == ',' || currChar == '\n') {
            curr = createStrn(currWord, 0);
            addStrnTail(tail, curr);
            if (*head == NULL) {
                *head = curr;
            }
            if (currChar == '\n') {
                processCSV(head, data);
                purgeStringList(head, 0);
                *head = *tail = NULL;
            }
            wordLen = 0;
        } else {
            swprintf(charToString, 2, L"%lc", currChar);

            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;

        }
        currChar = fgetwc(file);
    }

    free(head);
    free(tail);
    free(currWord);
    free(charToString);
    fclose(file);
    return data;
}

h_map *readFile(char *path) {
    h_map *data = mapBuild(0);
    wchar_t *currWord = malloc(sizeof(wchar_t) * 35), *prevWord = malloc(sizeof(wchar_t) * 35);
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2);

    if (currWord == NULL || charToString == NULL || prevWord == NULL) {
        printf("out of memory\n");
        return NULL;
    }

    swprintf(prevWord, 35, L"%lc", '.');
    mapPut(data, prevWord, createMainNode());
    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    while (currChar != WEOF) {
        if (wordLen == 0 && (currChar == ' ' || currChar == '\n')) {
            currChar = fgetwc(file);
            continue;
        }
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (currChar == ' ' || currChar == '\n' || currChar == '!' || currChar == '?' || currChar == '.') {
            if (wordLen != 0) {
                processFile(prevWord, currWord, data);
                wcscpy(prevWord, currWord);
            }
            if (currChar != ' ' && currChar != '\n') {
                wcscpy(currWord, charToString);
                processFile(prevWord, currWord, data);
                wcscpy(prevWord, currWord);
            }
            wordLen = 0;
        } else if (iswalpha(currChar) || iswalnum(currChar) || currChar == '\'') {
            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }

    fclose(file);
    free(currWord);
    free(prevWord);
    free(charToString);
    return data;
}


int writeCSVFile(h_map *fileContent, char *path) {
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

    StringNode **row, *curr;
    h_node *currNode;

    for (int i = 0; i < fileContent->capacity; i++) {
        currNode = fileContent->data[i];
        while (currNode != NULL) {
            row = buildRow(currNode);
            curr = *row;
            while (curr != NULL) {
                fwprintf(file, L"%ls", curr->word);
                curr = curr->next;
            }
            purgeStringList(row, 1);
            currNode = currNode->next;
        }
    }
    fclose(file);
    return 0;
}

void writeFile(StringNode **head, char *path) {
    FILE *file = fopen(path, "w");
    StringNode *curr = *head;

    while (curr != NULL) {
        fwprintf(file, L"%ls", curr->word);
        curr = curr->next;
    }

    fclose(file);
    purgeStringList(head, 1);
}


