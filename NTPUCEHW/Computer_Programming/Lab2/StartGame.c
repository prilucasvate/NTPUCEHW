#include <stdio.h>

int main()
{
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
    if(select==1){
        puts("Let's get started!!!");   
    }else if (select==2){
        puts("Good bye!!!");    
    }       
}
      
