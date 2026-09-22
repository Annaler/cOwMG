// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "cJSON.h"

#define getItem(obj, key) cJSON_GetObjectItemCaseSensitive((obj), (key))

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
} accList ;

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

    if (!cJSON_IsArray(root)){ fprintf(stderr, "root aint Array\n"); cJSON_Delete(root); }
    int size = cJSON_GetArraySize(root);
    if (size <= 0){ cJSON_Delete(root); return accs; }

    accs.account = (acc *)calloc(size, sizeof(acc));
    if (!accs.account){ fprintf(stderr, "mem out\n"); cJSON_Delete(root); return accs; }
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
        if (cJSON_IsString(sort)) snprintf(accs.account[i].sort, 10, "%s", sort->valuestring);
        if (cJSON_IsString(amount)) snprintf(accs.account[i].amount, 64, "%s", amount->valuestring);
        if (cJSON_IsString(mark)) snprintf(accs.account[i].mark, 32, "%s", mark->valuestring);
        if (cJSON_IsString(date)) snprintf(accs.account[i].date, 64, "%s", date->valuestring);
        i++;
    }
    cJSON_Delete(root);
    return accs;
}
int save_data(const char *path, const accList *accs){
    exit(2344);//占位
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
static size_t inputClean(char *str){
    fuck_lineBreak(str);
    fuck_spaceAndTab(str);
    return strlen(str);
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

static void clearStdin(void){
    int c;
    while ( (c = getchar()) != '\n' && c != EOF ){}
}
static bool getInput(char *buf, size_t size){
    if (fgets(buf, size, stdin) == NULL) {
        if (feof(stdin)) exit(3247);
        return false;
    }
    if (strchr(buf, '\n') == NULL) clearStdin();
    return true;
}

static void mait(char **str, const char *src){
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
bool in_dict(char **str, const char * const dict_key[], const char * const dict_value[]){
    if (*str == NULL) return false;
    for (int i = 0; dict_key[i] != NULL; i++){
        if (strcmp(*str, dict_key[i]) == 0){ mait(str, dict_value[i]); return true; }
    }
    for (int i = 0; dict_value[i] != NULL; i++){
        if (strcmp(*str, dict_value[i]) == 0) return true;
    }
    //not in
    free(*str); *str = NULL; return false;
}
bool in_iQuit(const char *buf){
    const char  * const iQuit[] = {"0", "q", "Q", "quit", "Quit", "退出", NULL}; 
    for (int i = 0; iQuit[i] != NULL; i++){
        if (strcmp(buf, iQuit[i]) == 0) return true;
    }
    return false;
}

bool validAmount(char *am){
    exit(2344);
}

static char getVal(){
    exit(2344);
}
//man!
void newAcc() {
    char input[100];//100应该够缓冲了
    bool quit = false;
    
    //load from data.json

    //getID
    
    //getSort
    const char * const sorts_key[] = {"1", "2", "3", "4", "5", "6", NULL};
    const char * const sorts_value[] = {"吃喝", "玩乐", "衣", "住", "行", "其他", NULL};
    char *sort = NULL;
    while (sort == NULL && !quit){
        printf("可选: 1. 吃喝 2. 玩乐 3.衣 4.住 5.行 6.其他\n");
        printf("请选择：");
        if (!getInput(input, sizeof(input))) continue;
        inputClean(input);
        if (in_iQuit(input)){ quit = true; break;}//how about not goto?
        //size_t len = strlen(input);
        if (strlen(input) == 0){
            printf("\nNot Sure?\n");
            continue;
        }
        if (!in_dict(&sort, sorts_key, sorts_value)){
            printf("\n您再斟酌斟酌\n");
            continue;
        };
    };

    //getAmount
    char *amount = NULL;
    while(amount == NULL && !quit){
        printf("Amount:");
        if (!getInput(input, sizeof(input))) continue;
        inputClean(input);
        if (in_iQuit(input)){ quit = true; break; }
        if (strlen(input) == 0){
            printf("\nno?\n");
            continue;
        }
        if (!validAmount(input)) continue;
        mait(&amount, input);
    }
    
    //getMark
    char *mark = NULL;
    while(mark == NULL && !quit){
        printf("备注：");
        if (!getInput(input, sizeof(input))) continue;
        //去除换行符
        inputClean(input);
        if (in_iQuit(input)){ quit = true; break;}
        //超载重来
        //size_t len = strlen(input);
        if (strlen(input) > (30)){
            printf("\ntoo long as a mark(max: 30 bytes).\n");
            continue;
        }
        mait(&mark, input);
    }

    //getDate
    char *date = NULL;
    while(date == NULL && !quit){
        printf("时间(默认当天):");
        getInput(input, sizeof(input));
        inputClean(input);
        if (in_iQuit(input)){ quit = true; break; }
        //if empty, today. else user's input
        if (strlen(input) == 0) getToday(input);
        if (!validDate(input)){
            printf("不支持的格式。\n");
            continue;
        }
        mait(&date, input);
    }
    
    //comp, append, save

    //...
    free(sort); sort = NULL;
    free(mark); mark = NULL;
    free(date); date = NULL;
    //may loop it
    //if (quit = true) break;
}


int main(){
    //load data from data.json
    //empty? create it
    const char *path = "data.json";

    if (exist_file(path) < 0){ perror("exist_file failed."); return 1; }

    char *json = read_file(path);
    if (!json){ perror("read_file failed."); return 1; }
    
    accList accounts = load_data(json);
    free(json);

    //mainMenu
    //want what
    //format choose
    char choose;
    switch(choose){
        case '1' : {newAcc(); break;}
        //...
    }
    //free(accs.account);
    return 0;
}

