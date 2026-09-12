#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>


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
void getToday(char **date){
    time_t t = time(NULL);
    struct tm *localDate_text = localtime(&t);
    
    char *m = malloc(11);
    if (!m) {perror("malloc failed.\n"); exit(69);}
    
    strftime(m, 11, "%Y-%m-%d", localDate_text);
    *date = m;
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

bool in_dict(char *str, const char * const dict_key[], const char * const dict_value[]){
    
    /*for i in dict_key[]{
        if (str == dict_key[i]){ str = dict_value[i]; return true;}
    }
    for i in dict_value[]{
        if (str == dict_value[i]) return true;
    }
    return false;*/
    exit(here_not_done_yet);
}

//man!
int main() {
    char input[100];//100应该够缓冲了
    const char  * const iQuit[] = {"0", "q", "Q", "quit", "Quit"}; 
    //load from data.json

    //getID
    
    //getSort
    const char * const sorts_key[] = {"1", "2", "3", "4", "5", "6", NULL};
    const char * const sorts_value[] = {"吃喝", "玩乐", "衣", "住", "行", "其他", NULL};
    char *sort = NULL;
    bool valid = false;
    while (!valid){
        printf("可选: 1. 吃喝 2. 玩乐 3.衣 4.住 5.行 6.其他\n");
        printf("请选择：");
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        inputClean(input);
        size_t len = strlen(input);
        if (len == 0){
            printf("\nNot Sure?\n");
            continue;
        }
        valid = in_dict(input, sorts_key, sorts_value);
        if (!valid){ printf("\n您再斟酌斟酌\n"); continue; }
        mait(&sort, input);
    };

    //getAmount


    //getMark
    char *mark = NULL;
    do{
        valid = false;
        printf("备注：");
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        //去除换行符
        inputClean(input);
        //超载重来
        size_t len = strlen(input);
        if (len > (30)){
            printf("\ntoo long as a mark(max: 30 bytes).\n");
            continue;
        }

        mait(&mark, input);
        valid = true;
    }while(!valid);

    //getDate
    char *date = NULL;
    while(true){
        printf("时间(默认当天):");
        fgets(input, sizeof(input), stdin);
        inputClean(input);
        //if empty, today. else user's input
        if (strlen(input) == 0){ getToday(&date); break; }
        else{
            if (validDate(input)){ mait(&date, input); break; }
            else printf("不支持的格式。\n");
        }
    }


    //...
    free(sort); sort = NULL;
    free(mark); mark = NULL;
    free(date); date = NULL;
    //may loop it
    return 44;
}
