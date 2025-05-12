#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Game.h"
#include "SingleComparison.h"
#include "Definitions.h"
int main()
{
    int position=0,card;
    int select=WelcomeMessage();
    int cards[52];
    for(int i=0;i<52;i++){
        cards[i]=i+1;
    }
    char select2;
    if(select==1){
        char fName[21];
        GetName(fName);
        printf("\nHello %s.",fName);
        puts(" Let's get started!!!");
        int loop=1;
        while(loop){
            MenuSelection(&select2);
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
                    position=0;
                    break;
                case 'B':
                    
                    printf("Deal card!!\n");
                    Deal1Card(cards,&position,&card);
                    struct Card dCard=MapCard(card);
                    printf("The dealer's card is %d(%d)\n",card,Face(card));
                    //-----------------------------
                    Deal1Card(cards,&position,&card);
                    struct Card pCard=MapCard(card);
                    printf("The player's card is %d(%d)\n",card,Face(card));
                    printf("%d cards left!!!!!!!\n\n",52-(position));
                    //---------------------------------------------
                    
                    if(Winner(dCard,pCard)==0){
                        printf("%s Lose:(\n",fName);
                    }else if(Winner(dCard,pCard)==1){
                        printf("%s Win :)\n",fName);
                    }
                    printf("------------------\n");
                    break;
                case 'C':
                    printf("See you next time!!\n");
                    loop=0;
                    break;
                default:
                    printf("Wrong selection!!\n");
                    break;
            }
        }
        
    }else if (select==2){
        puts("Good bye!!!");    
    }       
}
      





