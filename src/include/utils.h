#ifndef PROGETTO_DSTRUCT_H
#define PROGETTO_DSTRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <errno.h>
#include "map.h"

// definisco un enum per gestire in modo consistente gli errori "custom" dovuti alle specifiche del progetto
typedef enum error_codes_e {
    NO_ERROR = 0,
    BUILD_DATA_STRUCTURE_ERROR,
    STARTING_WORD_NOT_FOUND,
    EMPTY_FILE,
    MAX_ERROR_NUM = 4,
} error_codes_t;

// costruisco la mia "struttura dati" per la gestione degli errori
typedef struct result_s {
    int type; // contiene un codice di errore
    void *success; // contiene gli eventuali dati ritornati da una funzione X in caso termini senza errori
    void (*handler)(const char *);
    // puntatore ad una funzione che può gestire l'errore in type
    // In questo progetto la uso semplicemente per scegliere se chiamare "perror()" o "printErrorMessage()"
} result_t;

/*
 * Sia freqNode che mainNode sono pensati per l'utilizzo insieme ad h_node.
 * Esempio:
 * h_node *node => node->key = parola
 *                 node->val = freqNode/mainNode
*/
typedef struct freqNode {
    double frequency;
    int occurrences;
} FreqNode;

typedef struct mainNode {
    int nSuccessors;
    h_map *successors;
} MainNode;

void printErrorMessage(const char *prefix);
/*
 * dato in input un codice di errore, stampa il codice di errore "human readable",
 * nel caso in cui prefix != NULL la stringa printata ha come prefisso "{prefix}:{codice errore}"
 * input:
 *  const char *prefix => puntatore ad una stringa qualsiasi
*/

void capitalize(wchar_t *word);

/*
 * input:
 *  wchar_t *word => puntatore ad una stringa qualsiasi
 * output:
 * void result => non fornisce nessun output
 *
 * Modifica word sostituendo la prima lettera con il suo corrispettivo in maiuscolo
 * Esempio: "ciao" => "Ciao"
*/

void purgeCSVNodes(h_map *map);

/*
 * input:
 *  h_map *map => map è il puntatore ad un hashmap che rappresenta
 *                il contenuto del file CSV dato in input nel compito 2.
 * output:
 *  void result => non fornisce nessun output
 *
 * Libera SOLO la memoria occupata dai nodi delle parole presenti nell'hashmap
 * Attenzione! non libera tutta la memoria dell'hashmap
 * Esempio:
 * Sia h_node *node appartenente all'hashmap, purgeCSVNodes pulisce tutti i node->val (compreso il contenuto)
*/

FreqNode *createFreqNode();

/*
 * crea un nodo di tipo freqNode e restituisce un puntatore al suo indirizzo di memoria
*/


MainNode *createMainNode();
/*
 * crea un nodo di tipo freqNode e restituisce un puntatore al suo indirizzo di memoria
 */
#endif
