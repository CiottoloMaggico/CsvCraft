#ifndef PROGETTO_FILEUTIL_H
#define PROGETTO_FILEUTIL_H

#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include "textgen.h"
#include "utils.h"

wchar_t *nextCSVColumn(FILE *file);
/*
 * input:
 * FILE *file => stream del file csv da leggere
 * output:
 * wchar_t *result => caratteri letti dallo stream fino alla prima ",".
 *                    Nel caso il primo carattere letto è "\n", questo viene ritornato immediatamente per comunicare la fine della riga.
 *                    Nel caso in cui il primo carattere letto è WEOF ritorna un puntatore nullo.
*/


h_map *readAndBuildFileMap(char *path);
/*
 * input:
 * char *path => percorso del file in input
 * output:
 * h_map *result => puntatore ad un hashmap che rappresenta il contenuto del file TXT dato in input per il compito 1.
*/

h_map *readAndBuildCSVMap(char *path);
/*
 * input:
 * char *path => percorso del file in input
 * output:
 * h_map *result => puntatore ad un hashmap che rappresenta il contenuto del file CSV dato in input per il compito 2.
*/


int readCSVFile(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file CSV da leggere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che costruisce la struttura dati per eseguire il compito 2.
 * output:
 * int result => exit code
*/

void readFile(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file TXT da leggere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che costruisce la struttura dati per eseguire il compito 1.
 * output:
 * void result => non restituisce output
*/

int writeFileFromPipe(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file CSV da scrivere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che costruisce le righe del CSV per eseguire il compito 1.
 * output:
 * int result => exitcode, se uguale a 0 allora l'esecuzione è terminata senza errori
*/



#endif