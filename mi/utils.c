// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "utils.h"

void prinerr(const char *__format){ fprintf(stderr, "%s", __format); }
void printr(const char *__format, int times){
    if (__format == NULL ){ prinerr("printr,but print what?"); return; }
    for (int i = 0; i < times; i++){
        printf("%s", __format);
    }
    printf("\n");
}

int exist_file(const char *path){
    FILE *file = fopen(path, "rb");
    if (file){
        fseek(file, 0, SEEK_END);
        long len = ftell(file);
        fclose(file);
        if (len > 0) return 0;
    }

    file = fopen(path, "w");
    if (!file) return 404;
    fputs("[]", file);
    fclose(file);
    return 1;
}

char *read_file(const char *path){
    FILE *file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (len < 0){ fclose(file); return NULL; }

    char *buf = (char *)malloc(len + 1);
    if (!buf){ fclose(file); return NULL; }

    size_t end = fread(buf, 1, len, file);
    buf[end] = '\0';
    fclose(file);
    return buf;
}

//data from JSON到acc_list
accList load_data(const char *str){
    accList accs = { NULL, 0 };
    
    cJSON *root = cJSON_Parse(str);
    if (!root){ fprintf(stderr, "json analysing failed: %s\n", cJSON_GetErrorPtr()); return accs; }

    if (!cJSON_IsArray(root)){
        prinerr("root aint Array\n");
        cJSON_Delete(root);
        return accs;
    }
    int size = cJSON_GetArraySize(root);
    if (size <= 0){ cJSON_Delete(root); return accs; }

    accs.account = (acc *)calloc(size, sizeof(acc));
    if (!accs.account){ prinerr("mem out\n"); cJSON_Delete(root); return accs; }
    accs.len = size;

    int i = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root){
        cJSON *id = getItem(item, "id");
        cJSON *sort = getItem(item, "sort");
        cJSON *amount = getItem(item, "amount");
        cJSON *mark = getItem(item, "mark");
        cJSON *date = getItem(item, "date");

        if (cJSON_IsNumber(id)) accs.account[i].id = id->valueint;
        if (cJSON_IsString(sort)) snprintf(accs.account[i].sort, sizeof(accs.account[i].sort), "%s", sort->valuestring);
        if (cJSON_IsString(amount)) snprintf(accs.account[i].amount, sizeof(accs.account[i].amount), "%s", amount->valuestring);
        if (cJSON_IsString(mark)) snprintf(accs.account[i].mark, sizeof(accs.account[i].mark), "%s", mark->valuestring);
        if (cJSON_IsString(date)) snprintf(accs.account[i].date, sizeof(accs.account[i].date), "%s", date->valuestring);
        i++;
    }
    cJSON_Delete(root);
    return accs;
}

int save_data(const char *path, cJSON *root){
    //不重排,4real
    char *out = cJSON_Print(root);
    if (!out) return -1;

    char tmp[16];
    if (snprintf(tmp, sizeof(tmp), "%s.tmp", path) >= (int)sizeof(tmp)) goto fail;
    FILE *fp = fopen(tmp, "w");
    if (!fp) goto fail;
    if (fputs(out, fp) == EOF){ fclose(fp); remove(tmp); goto fail; }
    if (fflush(fp) != 0 || fclose(fp) != 0){ remove(tmp); goto fail; }
    free(out);
    if (rename(tmp, path) != 0){ remove(tmp); return -1;}
    return 0;
fail:
    free(out); return -1;
}

cJSON *comp_account(const accList *accs){
    cJSON *root = cJSON_CreateArray();
    if (!root) return NULL;
    for (int i = 0; i < accs->len; i++){
        cJSON *obj = cJSON_CreateObject();
        if (!obj){ cJSON_Delete(root); prinerr("cJSON_CreateObject failed"); return NULL; }
        cJSON_AddNumberToObject(obj, "id", accs->account[i].id);
        cJSON_AddStringToObject(obj, "date", accs->account[i].date);
        cJSON_AddStringToObject(obj, "sort", accs->account[i].sort);
        cJSON_AddStringToObject(obj, "amount", accs->account[i].amount);
        cJSON_AddStringToObject(obj, "mark", accs->account[i].mark);

        cJSON_AddItemToArray(root, obj);
    }
    return root;//用完记得cJOSN_Delete(root);
}

//去除换行符
static size_t fuck_lineBreak(char *str){
    if (str == NULL) return 0;
    
    size_t len = strlen(str);
    while ((len> 0 && str[len - 1] == '\n') ||
        (len > 0 && str[len - 1] == '\r')){
        str[len - 1] = '\0';
        len--;
    }
    
    return len;
}
//去除首尾空格&制表符
static char *fuck_spaceAndTab(char *str){
    if (str == NULL) return NULL;
    
    char *start = str;
    while (*start == ' ' || *start == '\t') start++;

    if (strlen(start) == 0){
        //if (start != str) memmove(str, start, 1);
        *str = '\0';
        return str;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t')){
        *end = '\0';
        end--;
    }
    if (start != str) memmove(str, start, strlen(start) + 1);
    return str;
}
size_t inputClean(char *str){
    fuck_lineBreak(str);
    fuck_spaceAndTab(str);
    return strlen(str);
}
void clearStdin(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF ){}
}
bool getInput(char *buf, size_t size){
    if (fgets(buf, size, stdin) == NULL) {
        if (feof(stdin)) exit(3247);
        return false;
    }
    if (strchr(buf, '\n') == NULL) clearStdin();
    return true;
}

bool validDate(const char *str){
    if (strlen(str) != 10) return false;
    for (int i = 0; i < 10; i++){
        if (i == 4 || i == 7){
            if (str[i] != '-')
                return false;//xxxx-xx-xx
        }else{//5th&8th must be^  ^ "-"
            if (str[i] < '0' || str[i] > '9')
                return false;}
    }

    unsigned short int year, month, day;
    if (sscanf(str, "%hu-%hu-%hu",&year, &month, &day) != 3) return false;
    if (month < 1 || month > 12) return false;
    unsigned short int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year %100 != 0) || (year %400 == 0)){
        days[2] = 29;
    }
    if (day < 1 || day > days[month]) return false;
    return true;
}
void getToday(char *buf){
    time_t t = time(NULL);
    struct tm *localDate_text = localtime(&t);
    strftime(buf, 11, "%Y-%m-%d", localDate_text);
}
int formatAmount(char *buf, size_t size){
    char *c = buf;
    if (*c == '-') c++;

    char *int_start = c;
    while(isdigit((unsigned char)*c)) c++;
    size_t int_len = (size_t)(c - int_start);

    size_t need = ((size_t)(int_start - buf) + int_len + 4);
    if (need > size) return -1;

    /*size_t deci_len = 0;
    char *deci_start = NULL;
    if (*c == "."){
        c++;
        deci_start = c;
        while (isdigit((unsigned char)*c) c++;
        deci_len = (size_t)(c - deci_start);
    }*/
    //总共就3种情况:0/1/2位小数，写毛线循环
    char d1 = '0', d2 = '0';
    if (*c == '.'){
        c++;
        if (isdigit((unsigned char)*c)){ d1 = *c; c++; }
        if (isdigit((unsigned char)*c)){ d2 = *c; c++; }
        if ( *c != '\0') return 1;
    }
    char *w = (int_start + int_len);
    *w++ = '.';
    *w++ = d1;
    *w++ = d2;
    *w++ = '\0';
    return 0;
}
bool validAmount(const char *str){
    if (str == NULL || *str == '\0') return false;
    
    const char *c = str;
    if (*c == '-') c++;
    if (!isdigit((unsigned char)*c)) return false;//小数点前至少一位
    do{ c++; } while(isdigit((unsigned char)*c));
    if (*c == '.'){
        c++;
        int d = 0;
        while (isdigit((unsigned char)*c)){ c++; d++; }
        if (d > 2) return false;
    }
    return true;
}

//我忘了这个函数何意味
/*static char getVal(){
    exit(2344);
}*/

void mait(char **str, const char *src){
    char *s = malloc(strlen(src) + 1);
    if (s == NULL){//may use perror? //yep
        perror("malloc failed.\n");
        exit(69);
    }
    strcpy(s, src);
    free(*str);
    *str = s;
}
//t在key则value，在value则value，二者皆true
const char *in_sort(
        char *str,
        const char * const dict_key[],
        const char * const dict_value[]){
    for (int i = 0; dict_key[i] != NULL; i++){
        if (strcmp(str, dict_key[i]) == 0) return dict_value[i];
    }
    for (int i = 0; dict_value[i] != NULL; i++){
        if (strcmp(str, dict_value[i]) == 0) return dict_value[i];
    }
    //not in
    return NULL;
}
bool in_iQuit(const char *buf){
    const char * const iQuit[] = {"0", "q", "Q", "quit", "Quit", "退出", "合账", NULL}; 
    for (int i = 0; iQuit[i] != NULL; i++){
        if (strcmp(buf, iQuit[i]) == 0) return true;
    }
    return false;
}