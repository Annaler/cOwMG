// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
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
        cJSON *acc = cJSON_CreateObject();
        if (!acc){ cJSON_Delete(root); perror("cJOOSN_CreateObject failed"); return NULL; }
        cJSON_AddNumberToObject(acc, "id", accs->account[i].id);
        cJSON_AddStringToObject(acc, "date", accs->account[i].date);
        cJSON_AddStringToObject(acc, "sort", accs->account[i].sort);
        cJSON_AddStringToObject(acc, "amount", accs->account[i].amount);
        cJSON_AddStringToObject(acc, "mark", accs->account[i].mark);

        cJSON_AddItemToArray(root, acc);
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
    while ((c = getchar()) != '\n' && c != EOF ){}
}
static bool getInput(char *buf){
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
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
bool in_sort(char **str, const char * const dict_key[], const char * const dict_value[]){
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

void formatAmount(char *buf){
    char *c = buf;
    bool isNegative = false;
    if (*c == '-'){ isNegative = true; c++; }

    char *int_start = c;
    while(isdigit((unsigned char)*c)) c++;
    size_t int_len = (size_t)(c - int_start);

    /*size_t deci_len = 0;
    char *deci_start = NULL;
    if (*c = "."){
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
        if (isdigit((unsigned char)*c)){ d1 = *c; c++; }
    }
    char *w = (int_start + int_len);
    *w++ = '.';
    *w++ = d1;
    *w++ = d2;
    *w++ = '\0';
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

void printr(const char *__format, int times){
    if (__format == NULL ) perror("printr,but print what?");
    for (int i = 0; i < times; i++){
        printf("%s", __format);
    }
    printf("\n");
}

//man!
void menu_new(const char *path, accList *accs) {
    char input[100];//100应该够缓冲了
    
    //getID
    int id = 1;
    if (accs->len >0) id = (accs->account[accs->len-1].id + 1);

    const char * const sorts_key[] = {"1", "2", "3", "4", "5", "6", NULL};
    const char * const sorts_value[] = {"吃喝", "玩乐", "衣", "住", "行", "其他", NULL};
    char *sort = NULL;
    while (sort == NULL){
        printf("可选: 1. 吃喝 2. 玩乐 3.衣 4.住 5.行 6.其他\n");
        printf("请选择：");
        if (!getInput(input)) continue;
        inputClean(input);
        if (in_iQuit(input)) goto quit;//how about not goto?
        //size_t len = strlen(input);
        if (strlen(input) == 0){
            printf("\nNot Sure?\n");
            continue;
        }
        if (!in_sort(&sort, sorts_key, sorts_value)){
            printf("\n您再斟酌斟酌\n");
            continue;
        };
    };

    //getAmount
    char *amount = NULL;//计算时用int64_t
    while(amount == NULL){
        printf("Amount:");
        if (!getInput(input)) continue;
        inputClean(input);
        if (in_iQuit(input)) goto quit;
        if (strlen(input) == 0){
            printf("\nno?\n");
            continue;
        }
        if (!validAmount(input)){ printf("格式错误"); continue; }
        formatAmount(input);
        mait(&amount, input);
    }
    
    //getMark
    char *mark = NULL;
    while(mark == NULL){
        printf("备注：");
        if (!getInput(input)) continue;
        //去除换行符
        inputClean(input);
        if (in_iQuit(input)) goto quit;
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
    while(date == NULL){
        printf("时间(默认当天):");
        getInput(input);
        inputClean(input);
        if (in_iQuit(input)) goto quit;
        //if empty, today. else user's input
        if (strlen(input) == 0) getToday(input);
        if (!validDate(input)){
            printf("不支持的格式。\n");
            continue;
        }
        mait(&date, input);
    }
    
    //comp, append, save
    acc account;
    account.id = id;
    strcpy(account.date, date);
    strcpy(account.sort, sort);
    strcpy(account.amount, amount);
    strcpy(account.mark, mark);
    //对accs操作
    acc *tmp = realloc(accs->account, (sizeof(acc) * (accs->len + 1)));
    if (!tmp){ perror("realloc failed"); goto quit; }
    accs->account = tmp;
    accs->account[accs->len] = account;
    accs->len++;   
    //对json操作一遍
    cJSON *root = comp_account(accs);//下方失败回滚//不，不如文件改写成功再改内存//不，过度复杂了那就
    if (!root){ accs->len--; perror("comp_account failed\n"); goto quit; }
    if (save_data(path,  root) != 0){ accs->len--; perror("save_data failed"); }
    cJSON_Delete(root);
   //...
quit:
    free(sort);
    free(amount);
    free(mark);
    free(date);
    //may loop it
    //if (quit = true) break;//dont,
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

    while (true){
        //mainMenu
        char input[10];//10应该够...吧
        //bool quit = false;
        printr("=", 44);
        printf("c · Oh where Money Go?");
        printr("=", 44);
        printf("1.新账(n) 2.阅账(v) 3.查账(f)\n4.划账(d) 5.统账(c) 6.改账(e)\n0.合账(q)\n");
        printr("=", 44);
        printf("掌柜的，怎么说？");
        getInput(input);
        inputClean(input);
        if (in_iQuit(input)) goto out;
        if (strlen(input) != 1){
            printf("\n这不对吧。\n");
            continue;
        }
        //want what
        /*if (input[0] == '1' || input[0] == 'n') choose = '1';
        else if (input[0] == '2' || input[0] == 'v') choose = '2';
        else if (input[0] == '3' || input[0] == 'f') choose = '3';
        else if (input[0] == '4' || input[0] == 'd') choose = '4';
        else if (input[0] == '5' || input[0] == 'c') choose = '5';
        else if (input[0] == '6' || input[0] == 'e') choose = '6';
        else{
            printf("");
            continue;
        }*/
        //format choose

        switch(input[0]){
            //新账
            case '1': case 'n': menu_new(path, &accounts); break;
            //阅账
            case '2': case 'v': break; 
            //查账
            case '3': case 'f': break;
            //划账
            case '4': case 'd': break;
            //统账
            case '5': case 'c': break;
            //改帐
            case '6': case 'e': break;
            //Man, what can I say?
            default: printf("\n这不对吧。\n"); continue;
        }
    }
out:
    free(accounts.account);
    return 0;
}

