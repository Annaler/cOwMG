// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include "../thirdparty/cJSON.h"

typedef struct{
    int id;
    char sort[10];
    char amount[32];
    char mark[64];
    char date[32];
} acc;

typedef struct {
    acc *account;
    int len;
} accList;

#define getItem(obj, key) cJSON_GetObjectItemCaseSensitive((obj), (key))

// io
void prinerr(const char *__format);
void printr(const char *__format, int times);

// file & data
int exist_file(const char *path);
char *read_file(const char *path);
accList load_data(const char *str);
int save_data(const char *path, cJSON *root);

// char opera
cJSON *comp_account(const accList *accs);
size_t inputClean(char *str);
void clearStdin(void);
bool getInput(char *buf, size_t size);

// var opera
bool validDate(const char *str);
void getToday(char *buf);
int formatAmount(char *buf, size_t size);
bool validAmount(const char *str);

// others
void mait(char **str, const char *src);
const char *in_sort(
        char *str,
        const char * const dict_key[],
        const char * const dict_value[]
        );
bool in_iQuit(const char *buf);

#endif // UTILS_H