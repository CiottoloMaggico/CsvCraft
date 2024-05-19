#include "fileutil.h"


void readCSVFile(char *path, int fd[]) {
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2), *currWord = malloc(sizeof(wchar_t) * 31);

    if (charToString == NULL || currWord == NULL) {
        printf("out of memory\n");
        return;
    }

    int wordLen = 0;
    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);

    while (currChar != WEOF) {
        if (currChar == ',' || currChar == '\n') {
            write(fd[1], currWord, sizeof(wchar_t) * 31);
            if (currChar == '\n') {
                write(fd[1], L"\n", sizeof(wchar_t) * 31);
            }
            wordLen = 0;
        } else {
            swprintf(charToString, 2, L"%lc", currChar);

            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;

        }
        currChar = fgetwc(file);
    }

    write(fd[1], L"--", sizeof(wchar_t) * 3);
    free(currWord);
    free(charToString);
    fclose(file);
    return;
}

void readFile(char *path, int fd[]) {
    wchar_t *charToString = malloc(sizeof(wchar_t) * 2), *currWord = malloc(sizeof(wchar_t) * 35);
    if (currWord == NULL || charToString == NULL) {
        printf("out of memory\n");
        return;
    }

    FILE *file = fopen(path, "r");
    wint_t currChar = fgetwc(file);
    int wordLen = 0;

    while (currChar != WEOF) {
        swprintf(charToString, 2, L"%lc", towlower(currChar));
        if (iswalnum(currChar) == 0) {
            if (wordLen != 0) {
                if (currChar == '\'') {
                    wcscat(currWord, charToString);
                    wordLen++;
                }
                write(fd[1], currWord, sizeof(wchar_t) * 35);
            }
            if (currChar == '!' || currChar == '?' || currChar == '.') {
                wcscpy(currWord, charToString);
                write(fd[1], currWord, sizeof(wchar_t) * 35);
            }
            wordLen = 0;
        } else {
            if (wordLen == 0) {
                wcscpy(currWord, charToString);
            } else {
                wcscat(currWord, charToString);
            }
            wordLen++;
        }
        currChar = fgetwc(file);
    }
    if (wordLen != 0) {
        write(fd[1], currWord, sizeof(wchar_t) * 35);
    }
    write(fd[1], L"--", sizeof(wchar_t) * 35);
    fclose(file);
    free(currWord);
    free(charToString);
    return;
}


int writeCSVFile(char *path, int fd[]) {
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return -2;
    }

    file = freopen(path, "a", file);

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return -2;
    }

    wchar_t *string = malloc(sizeof(wchar_t) * 40);

    while (1) {
        if (read(fd[0], string, (sizeof(wchar_t) * 40)) == -1) {
            printf("errore");
        }

        if (wcscmp(string, L"--") == 0) {
            break;
        }
        fwprintf(file, L"%ls", string);
    }

    fclose(file);
    free(string);
    return 0;
}

void writeFile(char *path, int fd[]) {
    FILE *file = fopen(path, "w");

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return;
    }

    file = freopen(path, "a", file);

    if (file == NULL) {
        printf("An error occurs while opening file.\n");
        return;
    }

    wchar_t *string = malloc(sizeof(wchar_t) * 40);

    while (1) {
        if (read(fd[0], string, (sizeof(wchar_t) * 40)) == -1) {
            printf("errore");
        }

        if (wcscmp(string, L"--") == 0) {
            break;
        }
        fwprintf(file, L"%ls", string);
    }

    fclose(file);
    free(string);
    return;
}


