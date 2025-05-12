
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Game.h"
#include "SingleComparison.h"
//#include "Definitions.h"
int Face(int num){
    int face=(num-1)%13+1;
    return face;
}

void ShuffleCards(int cards[]){
    int random=0;
    int temp=0;
    for(int i=51;i>0;i--){
        //srand(time(NULL));
        random=rand()%(i+1);
        temp=cards[i];
        cards[i]=cards[random];
        cards[random]=temp;
    }
}

void GetName(char name[]){
    char name1[21];
    char name2[10];
    printf("Please input the player's first name (10 characters at most):\n");
    scanf("%10s",name1);
    printf("Please input the player's last name (9 characters at most):\n");
    scanf("%9s",name2);
    strcat(name1," ");
    strcat(name1,name2);
    strcpy(name,name1);
}

int WelcomeMessage(){
    printf("Welcome to the Poker Game\n" );
    puts("Are you ready? 1)Yes 2)No");
    int select;
    scanf("%d", &select);
    fflush(stdin);
    while (!(select==1||select==2)){
        puts("Incorrect choice. Please enter 1 or 2.");
        scanf("%d", &select);
        fflush(stdin);
    }
    return select;
}
