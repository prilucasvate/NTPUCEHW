#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int intp[100]={0};
int j,oj,bestl=10000;
int ans[100];
int cons[10][10];

int path(int cost[10][10], int s, char p[100]){
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            cons[i][j] = cost[i][j]; // keep original

    p[0]='\0';
    int delNode,delDis=10000;
    //---------------------------------------------------start loop
    for (int ii = 0; ii < 10; ii++){
        int dist[10][10], fpath[10][10];
        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 10; ++j)
                cost[i][j] = cons[i][j];
        if((cost[s][ii]>=10000)||(cost[s][ii]==0)){
            continue;
        }
        
        delNode=ii;
        delDis=cost[s][ii];
        //printf("\n---------------this time break %d\n",delNode);
        //break nearest node
        cost[s][delNode]=10000;
        cost[delNode][s]=10000;
        //---------------------------------
        //B(delNode)-> A(s) floydWarshall
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                dist[i][j] = cost[i][j];
                if (i != j && cost[i][j] < 10000)
                    fpath[i][j] = j;    
                else
                    fpath[i][j] = -1; 
            }
        }

        for (int k = 0; k < 10; k++) {
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    if (dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        fpath[i][j] = fpath[i][k];  
                    }
                }
            }
        }
        if (dist[delNode][s] >= 10000) {
            continue;
        }
        //---------------------------------
        // for (int i = 0; i < 10; i++) {
        //     for (int j = 0; j < 10; j++) {
        //         printf(" %5d ",dist[i][j]);
        //     }
        //     printf("\n");
        // }
        // printf("path:\n");
        // for (int i = 0; i < 10; i++) {
        //     for (int j = 0; j < 10; j++) {
        //         printf(" %5d ",fpath[i][j]);
        //     }
        //     printf("\n");
        // }
        //---------------------------------
        
        
        
        intp[0]=s;
        intp[1]=delNode;
        int q=delNode; 
        //printf("  q: %d -> s: %d ",q,s);
        j=1;//counter
        while (q != s) {
            q = fpath[q][s];
            if (q == -1) {
                p[0]='n';
                return 0;
            }
            j++;
            intp[j]=q;
        }
        int leng=10000;
        leng=dist[delNode][s]+delDis;
        //printf("cycle length: %d\n",leng);
        //++++++++++++++++++++++++++++++++++++++++++++   
        if(bestl>leng){
            oj=j;
            bestl=leng;
            for (int i = 0; i < 10; ++i){
                ans[i] = intp[i];
            }
        }
        
    }
    if(bestl==10000){
        p[0]='n';
        return 0;
    }
    
    
    
    
    //---------make p
    char buffer[10];
    sprintf(buffer, "%d", ans[0]);
    strcat(p, buffer);
    for(int i =1; i<=oj; i++){
        sprintf(buffer, ",%d", ans[i]); 
        strcat(p, buffer); 
    }
    printf("shortest length: %d\n",bestl);
    return 0;  
}


int main()
{
    int n,i,j,s,d;
    char p[100];
   
//    int cost[10][10];
 
    // for(i=0;i<10;i++)
    // {
    //     for(j=0;j<10;j++)
    //     { 
    //        cost[i][j]=10000;
    //     }  
    // }

    // cost[0][0]=0;
    // cost[0][1]=1;
    // cost[0][3]=4;
    // cost[1][0]=1;
    // cost[1][1]=0;
    // cost[1][2]=3;
    // cost[1][4]=1;
    // cost[2][1]=3;
    // cost[2][2]=0;
    // cost[2][4]=1;
    // cost[2][5]=2;
    // cost[3][0]=4;
    // cost[3][3]=0;
    // cost[3][4]=1;
    // cost[4][1]=1;
    // cost[4][2]=1;
    // cost[4][3]=1;
    // cost[4][4]=0;
    // cost[5][2]=2;
    // cost[5][5]=0;

// 
#define INF 10000
/* 10000 代表不可達（∞）──剩下的 5~9 列只是填滿矩陣 */
int cost[10][10] = {
/*         0      1      2      3      4      5~9 (皆 10000) */
  /*0*/ {    0,     2,     1,     4, 10000, 10000, 10000, 10000, 10000, 10000 },
  /*1*/ {10000,     0,    -6, 10000, 10000, 10000, 10000, 10000, 10000, 10000 },
  /*2*/ {    7, 10000,     0, 10000, 10000, 10000, 10000, 10000, 10000, 10000 },
  /*3*/ {10000, 10000,     1,     0,     3, 10000, 10000, 10000, 10000, 10000 },
  /*4*/ {10000, 10000,     1, 10000,     0, 10000, 10000, 10000, 10000, 10000 },
  /*5*/ {10000, 10000, 10000, 10000, 10000,     0, 10000, 10000, 10000, 10000 },
  /*6*/ {10000, 10000, 10000, 10000, 10000, 10000,     0, 10000, 10000, 10000 },
  /*7*/ {10000, 10000, 10000, 10000, 10000, 10000, 10000,     0, 10000, 10000 },
  /*8*/ {10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,     0, 10000 },
  /*9*/ {10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,     0 }
};



    
    printf("Source: ");
    scanf("%d",&s);
    path(cost,s,p);
    

    if(p[0]=='n'){
    	printf("There is no path\n");
    }
    else{
        printf("The shortest path is %s",p); 
    }
    return 0;
      
}
