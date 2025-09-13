#include <stdio.h>
#include "Game.h"
int main()
{
    int select=WelcomeMessage();
    char select2;
    if(select==1){
        puts("Let's get started!!!");
        puts("(A) Shuffle Cards");
        puts("(B) Play Game");
        puts("(C) Exit Game");
        printf("Please select A, B, or C:\n");
        select2=getchar();
        switch (select2){
            case 'A':
            for (int i = 1; i <= 52; i++){
                if(i%13==0){
                    printf("13");
                    printf("\n");
                    continue;
                }
                printf("%d ",i%13);
                
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