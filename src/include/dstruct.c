#include "dstruct.h"

void purgeCSVNodes(h_map *map) {
    h_node *curr;
    h_node *insideCurr, *insidePrev;

    for (int i = 0; i < map->capacity; i++) {
        curr = map->data[i];
        while (curr != NULL) {
            insideCurr = curr->val;
            while (insideCurr != NULL) {
                insidePrev = insideCurr;
                insideCurr = insideCurr->next;
                free(insidePrev->key);
                free(insidePrev->val);
                free(insidePrev);
            }
            curr = curr->next;
        }
    }
}

void purgeStringList(StringNode **head, int freeHead) {
    StringNode *curr = *head, *prev;

    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }

    if (freeHead == 1) {
        free(head);
    }
}

FreqNode *createFreqNode() {
    FreqNode *result = malloc(sizeof(FreqNode));
    if (result == NULL) {
        printf("out of memory\n");
        exit(-1);
    }
    result->occurrences = result->frequency = 0;
    return result;
}

MainNode *createMainNode() {
    MainNode *result = malloc(sizeof(MainNode));
    if (result == NULL) {
        printf("out of memory");
        exit(-1);
    }
    result->nSuccessors = 0;
    result->successors = mapBuild(64);
    return result;
}

void capitalize(wchar_t *word) {
    word[0] = towupper(word[0]);
}

StringNode *createStrn(wchar_t *word, int format) {
    StringNode *result = malloc(sizeof(StringNode) + ((wcslen(word) + 5) * sizeof(wchar_t)));
    if (result == NULL) {
        printf("out of memory\n");
        exit(-1);
    }
    result->next = result->previous = NULL;
    wcscpy(result->word, word);

    if (format != 0) {
        wcscat(result->word, L" ");
        if (format == 2) {
            capitalize(result->word);
        }
    }

    return result;
}

void addStrnHead(StringNode **head, StringNode *node) {
    if (*head != NULL) {
        node->next = *head;
        (*head)->previous = node;
    }
    *head = node;
}

void addStrnTail(StringNode **tail, StringNode *node) {
    if (*tail != NULL) {
        node->previous = *tail;
        (*tail)->next = node;
    }
    *tail = node;
}

void printStrn(StringNode **head) {
    StringNode *curr = *head;
    while (curr != NULL) {
        if (curr->next == NULL) {
            wprintf(L"%ls -> NULL\n", curr->word);
        } else {
            wprintf(L"%ls -> ", curr->word);
        }
        curr = curr->next;
    }
}