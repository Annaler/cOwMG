// SPDX-FileCopyrightText: 2026 Annaler
// SPDX-License-Identifier: MPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mi/utils.h"
#include "mi/menu.h"

//man!
int main(){
    //load data from data.json
    //empty? create it
    const char *path = "data.json";

    if (exist_file(path) == 404){ prinerr("exist_file failed."); return 1; }

    char *json = read_file(path);
    if (!json){ perror("read_file failed."); return 1; }

    accList accounts = load_data(json);
    free(json);

    while (true){
        //mainMenu
        char input[10];//10应该够...吧
        //bool quit = false;
        printr("=", 44);
        printf("\nc · Oh where Money Go?\n");
        printr("=", 44);
        printf("\n1.新账(n) 2.阅账(v) 3.查账(f)\n4.划账(d) 5.统账(c) 6.改账(e)\n0.合账(q)\n");
        printr("=", 44);
        printf("掌柜的，怎么说？");
        getInput(input, sizeof(input));
        inputClean(input);
        if (in_iQuit(input)) goto out;
        if (strlen(input) != 1){
            printf("\n这不对吧。\n");
            continue;
        }
                
        switch(input[0]){
            //新账
            case '1': case 'n':
                if (menu_new(path, &accounts)){
                    prinerr("menu_new error, exit.\n");
                    goto out;
                } break;
            //阅账
            case '2': case 'v':
                printf("not done yet.\n");
                break; 
            //查账
            case '3': case 'f':
                printf("not done yet.\n");
                break;
            //划账
            case '4': case 'd': 
                printf("not done yet.\n");
                break;
            //统账
            case '5': case 'c':
                printf("not done yet.\n");
                break;
            //改帐
            case '6': case 'e':
                printf("not done yet.\n");
                break;
            //Man, what can I say?
            default: printf("\n这不对吧。\n"); continue;
        }
    }

out:
    free(accounts.account);
    return 0;
}

