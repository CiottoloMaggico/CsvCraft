#include "utils.h"

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