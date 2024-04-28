#include "textgen.h"

wchar_t *findNextWord(h_node *node) {
    h_node *curr = node;
    FreqNode *currFreq;

    double choice = (double) rand() / RAND_MAX;
    double lowBound = 0, upperBound = 0;

    while (curr != NULL) {
        currFreq = curr->val;
        lowBound = upperBound;
        upperBound += currFreq->frequency;

        if ((lowBound <= choice && choice <= upperBound) || curr->next == NULL) {
            return curr->key;
        }
        curr = curr->next;
    }
    return NULL;
}

wchar_t *findStartingWord(h_map *head) {
    h_node *startingPunctuation;
    int choice;

    do {
        choice = rand() % 3;
        switch (choice) {
            case 0:
                startingPunctuation = mapGet(head, L".");
                break;
            case 1:
                startingPunctuation = mapGet(head, L"?");
                break;
            case 2:
                startingPunctuation = mapGet(head, L"!");
                break;
        }
    } while (startingPunctuation == NULL);
    return findNextWord(startingPunctuation);
}

int generateText(h_map *fileContent, char *outputPath, int wordsNumber, wchar_t *startingWord) {
    if (startingWord == NULL) {
        startingWord = findStartingWord(fileContent);
    } else {
        if (mapGet(fileContent, startingWord) == NULL) {
            printf("Starting word not found.\n");
            return -1;
        }
    }

    StringNode **head = malloc(sizeof(StringNode *)), **tail = malloc(sizeof(StringNode *));

    if (head == NULL || tail == NULL) {
        printf("out of memory\n");
        return -1;
    }

    StringNode *currWordNode = createStrn(startingWord, 2);
    h_node *currFrequencies;
    wchar_t *currWord = startingWord, *nextWord;
    *head = *tail = currWordNode;

    for (int i = 0; i < wordsNumber - 1; i++) {
        currFrequencies = mapGet(fileContent, currWord);
        nextWord = findNextWord(currFrequencies);

        if (wcscmp(currWord, L"?") == 0 || wcscmp(currWord, L"!") == 0 ||
            wcscmp(currWord, L".") == 0) {
            currWordNode = createStrn(nextWord, 2);
        } else {
            currWordNode = createStrn(nextWord, 1);
        }

        addStrnTail(tail, currWordNode);
        currWord = nextWord;
    }

    purgeCSVNodes(fileContent);
    freeMap(fileContent);
    writeFile(head, outputPath);
    free(tail);
    return 0;
}

StringNode **buildRow(h_node *node) {
    StringNode *curr, **head = malloc(sizeof(StringNode *)), **tail = malloc(sizeof(StringNode *));
    wchar_t *currWord = malloc(sizeof(wchar_t) * 40);

    if (head == NULL || tail == NULL || currWord == NULL) {
        printf("Out of memory\n");
        exit(-1);
    }
    *head = *tail = createStrn(node->key, 0);
    MainNode *mainNode = node->val;
    FreqNode *currFreq;
    h_map *successors = mainNode->successors;
    h_node *currNode;

    for (int i = 0; i < successors->capacity; i++) {
        currNode = successors->data[i];
        while (currNode != NULL) {
            currFreq = currNode->val;
            currFreq->frequency = (double) currFreq->occurrences / mainNode->nSuccessors;
            swprintf(currWord, 40, L",%ls,%.4f", currNode->key, currFreq->frequency);
            curr = createStrn(currWord, 0);
            addStrnTail(tail, curr);
            free(currFreq);
            currNode = currNode->next;
        }
    }

    curr = createStrn(L"\n", 0);
    addStrnTail(tail, curr);
    freeMap(mainNode->successors);
    free(mainNode);
    free(currWord);
    free(tail);
    return head;
}

void processCSV(StringNode **head, h_map *data) {
    h_node **hHead = malloc(sizeof(h_node *)), *currNode;
    if (head == NULL) {
        printf("Out of memory.\n");
        exit(-1);
    }

    FreqNode *currFreq;
    StringNode *curr = (*head)->next;
    wchar_t *tmp, *mainWord;
    *hHead = NULL;

    while (curr != NULL && curr->next != NULL) {
        mainWord = curr->word;
        curr = curr->next;
        currFreq = createFreqNode();
        currFreq->frequency = wcstod(curr->word, &tmp);
        currNode = hNodeBuild(mainWord, currFreq);
        hNodeAdd(hHead, currNode);
        curr = curr->next;
    }
    mapPut(data, (*head)->word, *hHead);
    free(hHead);
}

void processFile(wchar_t *prevWord, wchar_t *currWord, h_map *data) {
    MainNode *curr = mapGet(data, prevWord);
    FreqNode *currFreqNode = mapGet(curr->successors, currWord);

    if (currFreqNode == NULL) {
        currFreqNode = createFreqNode();
        mapPut(curr->successors, currWord, currFreqNode);
    }

    curr->nSuccessors++;
    currFreqNode->occurrences++;

    curr = mapGet(data, currWord);
    if (curr == NULL) {
        mapPut(data, currWord, createMainNode());
    }
}

