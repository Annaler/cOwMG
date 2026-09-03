#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>


//去除换行符
size_t fuck_lineBreak(char *str){
    if (str == NULL) return 0;
    
    size_t len = strlen(str);
    if ((len> 0 && str[len - 1] == '\n') ||
        (len > 0 && str[len - 1] == '\r')){
        str[len - 1] = '\0';
        len--;
    }
    
    return len;
}
//去除首尾空格&制表符
char *fuck_spaceAndTab(char *str){
    if (str == NULL) return NULL;
    
    char *before = str;
    while (*before == ' ' || *before == '\t') before++;

    char *behind = before + strlen(before) - 1;
    while (behind > before && (*behind == ' ' || *behind == '\t')){
        *behind = '\0';
        behind--;
    }

    if (before != str){
        memmove(str, before, strlen(before) + 1);
    }

    return str;
}
size_t inputClean(char *str){
    fuck_spaceAndTab(str);
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
    sscanf(str, "%hu-%hu-%hu",&year, &month, &day);
    if (month < 1 || month > 12) return false;
    unsigned short int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year %100 != 0) || (year %400 == 0)){
        days[2] = 29;
    }
    if (day < 1 || day > days[month]) return false;
    return true;
}
void getToday(char *date){
    time_t t = time(NULL);
    struct tm *dateText = localtime(&t);
    strftime(date, 10, "%Y-%m-%d", dateText);
}

void mait(char *str, size_t len){
    str = (char *)malloc(len + 1);
        if (str == NULL){
            printf("malloc failed\n");
            return 69;
        }
}

bool in_dict(char str, char *dict_key[], char *dict_value[]){
    for i in dict_key[]{
        if (str == dict_key[i]) str = dict_value[i] return true;
    }
    for i in dict_value[]{
        if (str == dict_value[i]) return true;
    }
    return false;
}

//man!
int main() {
    char input[100];//100应该够缓冲了
    const char  * const iQuit[] = ["0", "q", "Q", "quit", "Quit"] 
    //load from data.json

    //getID
    
    //getSort
    const char * const sorts_key[] = {"1", "2", "3", "4", "5", "6"};
    const char * const sorts_value[] = {"吃喝", "玩乐", "衣", "住", "行", "其他"};
    char *sort = NULL;
    bool valid = false;
    while (!valid){
        printf("可选: 1. 吃喝 2. 玩乐 3.衣 4.住 5.行 6.其他\n");
        printf("请选择：");
        fgets(input, sizeof(input), stdin);
        inputClean(input);
        size_t len = strlen(input);
        if (len == 0){
            printf{"Not Sure?\n"};
            continue;
        }
        valid = in_dict(input, sorts_key, sorts_value);
        if (!valid) printf("您再斟酌斟酌");
        mait(sort, len);
        strcpy(sort, input);
    };

    //getAmount


    //getMark
    char *mark = NULL;
    bool valid = false;
    while (!valid){
        printf("备注：");
        fgets(input, sizeof(input), stdin);
        //去除换行符
        inputClean(input);
        //超载重来
        size_t len = strlen(input);
        if (len > (30 + 1)){
            printf("\ntoo long as a mark(max: 30 bytes).\n");
            continue;
        }

        mait(mark, len);
        strcpy(mark, input);
        valid = true;
    }

    //getDate
    char *date = NULL;
    while(1){
        printf("时间(默认当天):");
        fgets(input, sizeof(input), stdin);
        inputClean(input);
        //if empty, today. else user's input
        if (strlen(input) == 0){
            getToday(date);
            break;
        }else{
            if (validDate(input)){
                date = input;
                break;}}
    }


    //...
    free(sort);
    free(mark);
    sort, mark, date = NULL;
    //may loop it
    return 44;
}
