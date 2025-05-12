#include <stdio.h>

int main()
{
    int select;
    printf("%s","Welcome to the Poker Game\n" );
    puts("Are you ready? 1)Yes 2)No");
    scanf("%d", &select);
    printf("Your choise is " );
    printf("%d", select);
}