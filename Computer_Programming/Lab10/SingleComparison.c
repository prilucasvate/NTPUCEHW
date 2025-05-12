
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Game.h"
#include "SingleComparison.h"
#include "Definitions.h"

void MenuSelection(char *select2){
    puts("(A) Shuffle Cards");
    puts("(B) Play Game");
    puts("(C) Exit Game\n");
    printf("Please select A, B, or C:\n");
    fflush(stdin);
    *select2=getchar();
}

void Deal1Card(const int* const cards,int *position, int *card){
    
    if(*position==52){
        *position=0;
    }
    *card=cards[*position];
    *position+=1;
}
struct Card MapCard(int cd){
    struct Card c;
    if(cd>0&&cd<14){
        c.face=Face(cd);
        c.suit='S';
    }else if(cd>13&&cd<27){
        c.face=Face(cd);
        c.suit='H';
    }else if(cd>26&&cd<40){
        c.face=Face(cd);
        c.suit='D';
    }else if(cd>39&&cd<53){
        c.face=Face(cd);
        c.suit='C';
    }
    return c;
}
int Winner (struct Card dCard,struct Card pCard){
    int res=2;
    
    if(dCard.face>pCard.face){
        res=0;
    }else if(dCard.face<pCard.face){
        res=1;
    }
    else if(dCard.face==pCard.face){
        if(dCard.suit>pCard.suit){
            res=0;
        }else if (dCard.suit<pCard.suit){
            res=1;
        }
        
    }
    
    return res;
}