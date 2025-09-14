#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#define TOTAL_UNICODE_NUM 65536

typedef struct {
    wchar_t character;
    int count;
    double probability;
} CharInfo;

void countUnicode(FILE *f);
void calculateProbability(CharInfo charInfo[], int totalCharacters);
void sortByCount(CharInfo charInfo[]);
void printResult(CharInfo charInfo[], int totalCharacters);

int numUnicode[TOTAL_UNICODE_NUM] = {0};
int totalCharacters = 0;

/* ---- qsort  ---- */
static int cmpCharInfo(const void *a, const void *b) {
    const CharInfo *x = (const CharInfo*)a;
    const CharInfo *y = (const CharInfo*)b;
    if (x->count != y->count) {
        /* y - x：descending */
        return (y->count - x->count);
    }
    /* ascending */
    if (x->character < y->character) return -1;
    if (x->character > y->character) return 1;
    return 0;
}

int main(void)
{
    setlocale(LC_ALL, "");

    FILE *f = stdin;

    countUnicode(f);

    CharInfo charInfo[TOTAL_UNICODE_NUM];
    for (int i = 0; i < TOTAL_UNICODE_NUM; i++) {
        charInfo[i].character = (wchar_t)i;
        charInfo[i].count = numUnicode[i];
        charInfo[i].probability = 0.0;
    }

    calculateProbability(charInfo, totalCharacters);

    /* sort */
    sortByCount(charInfo);

    printResult(charInfo, totalCharacters);

    return 0;
}

void countUnicode(FILE *f) {
    wint_t ch;
    while ((ch = fgetwc(f)) != WEOF) {
        if (ch >= 0 && ch < TOTAL_UNICODE_NUM) {
            numUnicode[ch]++;
            totalCharacters++;
        }
    }
}

void calculateProbability(CharInfo charInfo[], int totalCharacters) {
    if (totalCharacters <= 0) return; 
    for (int i = 0; i < TOTAL_UNICODE_NUM; i++) {
        if (charInfo[i].count > 0) {
            charInfo[i].probability = (double)charInfo[i].count / (double)totalCharacters;
        }
    }
}

void sortByCount(CharInfo charInfo[]) {
    qsort(charInfo, TOTAL_UNICODE_NUM, sizeof(CharInfo), cmpCharInfo);
}

void printResult(CharInfo charInfo[], int totalCharacters) {
    (void)totalCharacters; /* 目前未使用，但保留參數以符合原型 */
    for (int i = 0; i < TOTAL_UNICODE_NUM; i++) {
        if (charInfo[i].count > 0) {
            if (charInfo[i].character == L'\n') {
                printf("'\\n',%d,%.15f\n", charInfo[i].count, charInfo[i].probability);
            } else if (charInfo[i].character == L'\r') {
                printf("'\\r',%d,%.15f\n", charInfo[i].count, charInfo[i].probability);
            } else if (charInfo[i].character == L',') {
                printf("\"%lc\",%d,%.15f\n", charInfo[i].character, charInfo[i].count, charInfo[i].probability);
            } else if (charInfo[i].character == L'\t') {
                printf("'\\t',%d,%.15f\n", charInfo[i].count, charInfo[i].probability);
            } else {
                printf("'%lc',%d,%.15f\n", charInfo[i].character, charInfo[i].count, charInfo[i].probability);
            }
        }
    }
}
