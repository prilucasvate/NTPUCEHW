#include <stdio.h>
#include <stdlib.h>
#include "Game.h"
//#include <time.h>
int main()
{
    int select=WelcomeMessage();
    int cards[52];
    for(int i=0;i<52;i++){
        cards[i]=i+1;
    }
    char select2;
    if(select==1){
        char name[21];
        GetName(name);
        printf("\nHello %s.",name);
        puts(" Let's get started!!!");
        puts("(A) Shuffle Cards");
        puts("(B) Play Game");
        puts("(C) Exit Game");
        printf("Please select A, B, or C:\n");
        fflush(stdin);
        select2=getchar();
        switch (select2){
            case 'A':
                printf("Cards shuffled!!");
                ShuffleCards(cards);
                puts("");
                for(int j=0;j<52;j++){
                    printf("%d(%d) ",cards[j],Face(cards[j]));
                    if(j%13==12){
                        puts("");
                    }
                }
                break;
            case 'B':
                printf("Deal card!!\n");
                break;
            case 'C':
                printf("See you next time!!\n");
                break;
            default:
                printf("Wrong selection!!\n");
                break;
        }
    }else if (select==2){
        puts("Good bye!!!");    
    }       
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
void GetName(char name[]){
    printf("Please input the player's name (20 characters at most):\n");
    scanf("%20s",name);
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
int Face(int num){
    int face=(num-1)%13+1;
    return face;
}