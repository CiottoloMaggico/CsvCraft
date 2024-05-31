#ifndef PROGETTO_TEXTGEN_H
#define PROGETTO_TEXTGEN_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <float.h>
#include <wctype.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include "utils.h"
#include "fileutil.h"
#include "map.h"

extern const int MAX_WORD_LEN;

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

result_t findStartingWord(h_map *fileContent);

/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file CSV dato in input nel compito 2.
 * output:
 * wchar_t *result => puntatore di tipo wchar_t ad una stringa il cui valore è scelto casualmente ed appartiene
 *                    all'insieme dei successori delle parole: "?", "!", "." .
*/

result_t generateText(h_map *fileContent, int wordsNumber, wchar_t *startingWord, bool_t singleProcessMode, ...);

/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file CSV dato in input nel compito 2.
 * int wordsNumber => numero di parole da generare
 * wchar_t startingWord => parola con cui iniziare la generazione,
 *            se non fornita ne verrà scelta una in modo casuale usando findStartingWord()
 * int mode => numero intero, 0 o 1, che specifica se eseguire il task in modalità multi-processo o singolo-processo
 * Se mode == 0:
 *  deve essere fornito come argomento aggiuntivo:
 *      int *fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *                  processo che scrive le parole generate sul file di output del compito 2
 * Se mode == 1:
 * deve essere fornito come argomento aggiuntivo:
 *      char *path => nome del file TXT da scrivere
 * output:
 * result_t result => comunico al chiamante se l'esecuzione della funzione è terminata con errori o no.
 * se result.type != NO_ERROR => l'esecuzione è terminata con errori
*/

result_t buildFileRows(h_map *fileContent, bool_t singleProcessMode, ...);

/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file TXT dato in input nel compito 1.
 * int mode => numero intero, 0 o 1, che specifica se eseguire il task in modalità multi-processo o singolo-processo
 * Se mode == 0:
 *  deve essere fornito come argomento aggiuntivo:
 *      int *fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *                  processo che scrive le righe generate sul file di output del compito 1
 * Se mode == 1:
 *  deve essere fornito come argomento aggiuntivo:
 *      char *path => nome del file CSV da scrivere
 * output:
 * result_t result => comunico al chiamante se l'esecuzione della funzione è terminata con errori o no.
 * se result.type != NO_ERROR => l'esecuzione è terminata con errori
*/

int buildRow(h_node *node, bool_t singleProcessMode, ...);

/*
 * input:
 * h_node *node => puntatore ad un nodo appartenente all'hashmap che rappresenta il contenuto del file TXT
 *                 dato in input nel compito 1 e che contiene le informazioni necessarie a costruire la riga del CSV
 *                 di output per la parola node->key.
 *                 node->key: rappresenta la prima parola della riga
 *                 node->val: è un nodo di tipo MainNode che contiene le informazione circa i successori di node->key
 * int mode => numero intero, 0 o 1, che specifica se eseguire il task in modalità multi-processo o singolo-processo
 * Se mode == 0:
 *  deve essere fornito come argomento aggiuntivo:
 *      int *fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *                  processo che scrive le righe generate sul file di output del compito 1
 * Se mode == 1:
 *  deve essere fornito come argomento aggiuntivo:
 *      FILE *file => output stream su cui scrivere la riga
 * output:
 * int result => comunico al chiamante un codice di errore per segnalare se l'esecuzione della funzione è terminata con errori o no.
 * se result != NO_ERROR => l'esecuzione della funzione è terminata con errori
*/

MainNode *addToData(h_map *fileContent, wchar_t **firstWord, MainNode *currWord, wchar_t *nextWord);

/*
 * input:
 * h_map *fileContent => fileContent è il puntatore ad un hashmap che rappresenta
 *                       il contenuto del file TXT dato in input nel compito 1.
 *
 * wchar_t **firstWord => nel caso in cui currWord == NULL,
 *                        firstWord viene settato per puntatare alla chiave del
 *                        nodo relativo a nextWord in fileContent.
 *                        Altrimenti, non si effettua nessuna operazione con firstWord
 * MainNode *currWord => puntatore al MainNode relativo al nodo a cui va aggiunto/aggiornato il successore "nextWord"
 * wchar_t *nextWord => puntatore ad una stringa che va aggiunta alla struttura dati fileContent se non già presente
 *
 * output:
 * MainNode *result => puntatore al MainNode relativo a nextWord che è stato anche inserito all'interno di fileContent.
*/

result_t processCSV(int fd[]);

/*
 * input:
 * int fd[] => array di file descriptors relativi alla pipe utilizzata per comunicare con il processo che legge le parole dal CSV
 *           dato in input per il compito 2.
 * output:
 * result_t result => comunico al chiamante se l'esecuzione della funzione è terminata con errori o no.
 * Se result.type == NO_ERROR => result.success contiene il puntatore ad un hashmap che rappresenta il contenuto del file CSV dato in input per il compito 2.
 * Altrimenti, l'esecuzione della funzione è terminata con un errore che può essere "gestito" utilizzando la funzione puntata da result.handler
*/

result_t processFile(int fd[]);
/*
 * input:
 * int fd[] => array di file descriptors relativi alla pipe utilizzata per comunicare con il processo che legge le parole dal CSV
 *           dato in input per il compito 1.
 * output:
 * h_map *result => comunico al chiamante se l'esecuzione della funzione è terminata con errori o no.
 * Se result.type == NO_ERROR => result.success contiene il puntatore ad un hashmap che rappresenta il contenuto del file TXT dato in input per il compito 1.
 * Altrimenti, l'esecuzione della funzione è terminata con un errore che può essere "gestito" utilizzando la funzione puntata da result.handler
*/

#endif