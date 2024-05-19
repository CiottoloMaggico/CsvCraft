#ifndef PROGETTO_TEXTGEN_H
#define PROGETTO_TEXTGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <float.h>
#include <wctype.h>
#include "utils.h"
#include "fileutil.h"
#include "map.h"

wchar_t *findNextWord(h_node *node);
/*
 * input:
 *  h_node *node => node è il puntatore alla testa della linked-list dei successori di una data parola (non specificata)
 *                  node->key: rappresenta il successore
 *                  node->val: è un nodo di tipo FreqNode che contiene le informazione circa la frequenza di node->key come successore
 * output:
 *  wchar_t *result => puntatore di tipo wchar_t ad una stringa il cui valore è scelto casualmente
 *                     in base alle frequenze dei nodi della linked-list.
 *
*/


wchar_t *findStartingWord(h_map *fileContent);
/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file CSV dato in input nel compito 2.
 * output:
 * wchar_t *result => puntatore di tipo wchar_t ad una stringa il cui valore è scelto casualmente ed appartiene
 *                    all'insieme dei successori delle parole: "?", "!", "." .
*/

int generateText(h_map *fileContent, int wordsNumber, wchar_t *startingWord, int fd[]);
/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file CSV dato in input nel compito 2.
 * int wordsNumber => numero di parole da generare
 * wchar_t startingWord => parola con cui iniziare la generazione,
 *            se non fornita ne verrà scelta una in modo casuale usando findStartingWord()
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che scrive le parole generate sul file di output del compito 2
 * output:
 * int result => exitCode della funzione, se viene ritornato 0 allora l'esecuzione è andata a buon fine
*/

void buildFileRows(h_map *fileContent, int fd[]);
/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file TXT dato in input nel compito 1.
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che scrive le parole generate sul file di output del compito 1.
 * output:
 * void result => non fornisce nessun output.
*/


void buildRow(h_node *node, int fd[]);
/*
 * input:
 * h_node *node => puntatore ad un nodo appartenente all'hashmap che rappresenta il contenuto del file TXT
 *                 dato in input nel compito 1 e che contiene le informazioni necessarie a costruire la riga del CSV
 *                 di output per la parola node->key.
 *                 node->key: rappresenta la prima parola della riga
 *                 node->val: è un nodo di tipo MainNode che contiene le informazione circa i successori di node->key
 *
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che scrive le parole generate sul file di output del compito 1.
 * output:
 * void result => non fornisce nessun output.
*/

void addToData(h_map *fileContent, wchar_t *prevWord, wchar_t *currWord);
/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file TXT dato in input nel compito 1.
 *
 * wchar_t *prevWord => puntatore ad una stringa a cui va aggiunto/aggiornato il successore currWord
 * wchar_t *currWord => puntatore ad una stringa che va aggiunta alla struttura dati fileContent se non già presente
 *
 * output:
 * void result => non fornisce nessun output.
*/

h_map *processCSV(int fd[]);
/*
 * input:
 * int fd[] => array di file descriptors relativi alla pipe utilizzata per comunicare con il processo che legge le parole dal CSV
 *           dato in input per il compito 2.
 * output:
 * h_map *result => puntatore ad un hashmap che rappresenta il contenuto del file CSV dato in input per il compito 2.
*/

h_map *processFile(int fd[]);
/*
 * input:
 * int fd[] => array di file descriptors relativi alla pipe utilizzata per comunicare con il processo che legge le parole dal CSV
 *           dato in input per il compito 1.
 * output:
 * h_map *result => puntatore ad un hashmap che rappresenta il contenuto del file TXT dato in input per il compito 1.
*/

#endif