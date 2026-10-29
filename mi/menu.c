// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "menu.h"

int menu_new(const char *path, accList *accs) {
    int ret = 0;
    char input[100];//100应该够缓冲了
    
    int id = 1;
    char *mark = NULL;
    char *amount = NULL;//计算时用int64_t
    char *date = NULL;
    char *sort = NULL;

    //getID
    if (accs->len >0) id = (accs->account[accs->len-1].id + 1);

    const char * const sorts_key[] = {"1", "2", "3", "4", "5", "6", NULL};
    const char * const sorts_value[] = {"吃喝", "玩乐", "衣", "住", "行", "其他", NULL};
    while (sort == NULL){
        printf("可选: 1. 吃喝 2. 玩乐 3.衣 4.住 5.行 6.其他\n");
        printf("请选择：");
        if (!getInput(input, sizeof(input))) continue;
        inputClean(input);
        if (in_iQuit(input)) goto quit;//how about not goto?
        //size_t len = strlen(input);
        if (strlen(input) == 0){
            printf("\nNot Sure?\n");
            continue;
        }
        const char *v = in_sort(input, sorts_key, sorts_value);
        if (!v){ printf("\n您再斟酌斟酌\n"); continue; }
        mait(&sort, v);
    };

    //getAmount
    while(amount == NULL){
        printf("Amount:");
        if (!getInput(input, sizeof(input))) continue;
        inputClean(input);
        if (in_iQuit(input)) goto quit;
        if (strlen(input) == 0){
            printf("\nno?\n");
            continue;
        }
        if (!validAmount(input)){ printf("格式错误\n"); continue; }
        int how = (formatAmount(input, sizeof(input)));
        if (how == 1){
            printf("that's too right.");
            continue;
        }
        if (how == -1){ ret = -1; goto quit; }
        mait(&amount, input);
    }
    
    //getMark
    while(mark == NULL){
        printf("备注：");
        if (!getInput(input, sizeof(input))) continue;
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
    while(date == NULL){
        printf("时间(默认当天):");
        if (!getInput(input, sizeof(input))) continue;
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
    if (!tmp){ perror("realloc failed"); ret = -1; goto quit; }
    accs->account = tmp;
    accs->account[accs->len] = account;
    accs->len++;   
    //对json操作一遍
    cJSON *root = comp_account(accs);//下方失败回滚
                                     //不，不如文件改写成功再改内存
                                     //不，过度复杂了那就
    if (!root){ accs->len--; prinerr("comp_account failed\n"); ret = -1; goto quit; }
    if (save_data(path,  root) != 0){
        accs->len--; prinerr("save_data failed"); ret = -1;
    }
    cJSON_Delete(root);
   //...
quit:
    free(sort);
    free(amount);
    free(mark);
    free(date);
    return ret;
    //may loop it
    //if (quit = true) break;//dont,
}
