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

void readCSVFile(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file CSV da leggere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che costruisce la struttura dati per eseguire il compito 2.
 * output:
 * void result => non restituisce output
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

int writeCSVFile(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file CSV da scrivere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che costruisce le righe del CSV per eseguire il compito 1.
 * output:
 * int result => exitcode, se uguale a 0 allora l'esecuzione è terminata senza errori
*/

void writeFile(char *path, int fd[]);
/*
 * input:
 * char *path => nome del file TXT da scrivere
 * int fd[] => array di file descriptors relativi alla pipe usata per comunicare con il
 *             processo che calcola le parole da scrivere nel TXT per eseguire il compito 2.
 * output:
 * void result => non restituisce output
*/


#endif