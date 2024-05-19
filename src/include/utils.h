#ifndef PROGETTO_DSTRUCT_H
#define PROGETTO_DSTRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include "map.h"

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
