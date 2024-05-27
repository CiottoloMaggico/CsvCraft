#include "utils.h"

// ERROR_STRINGS è un array che contiene le traduzioni leggibili dei codici di errore da me definiti
const char *ERROR_STRINGS[] = {
        "An error occurs trying to build the data structure needed to solve the task\n",
        "The starting word provided isn't in the csv provided\n",
        "The input file provided is empty\n",
};

void printErrorMessage(const char *prefix) {
    if (errno > 0 && errno < MAX_ERROR_NUM) {
        (prefix != NULL) ?
        printf("%s: %s", prefix, ERROR_STRINGS[errno - 1]) :
        printf("%s", ERROR_STRINGS[errno - 1]);
    }
}

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
        perror("fatal error, crashing");
        exit(errno);
    }
    result->occurrences = result->frequency = 0;
    return result;
}

MainNode *createMainNode() {
    MainNode *result = malloc(sizeof(MainNode));
    if (result == NULL) {
        perror("fatal error, crashing");
        exit(errno);
    }
    result->nSuccessors = 0;
    result->successors = mapBuild(64);
    return result;
}

void capitalize(wchar_t *word) {
    word[0] = towupper(word[0]);
}