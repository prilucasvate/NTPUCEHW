#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>


int intp[100]={0};
int j,oj;
int ans[100];
int cons[10][10];
int path(int cost[10][10], int s, char p[100])
{
    int bestl=10000;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 10; ++j)
            cons[i][j] = cost[i][j];

    p[0]='\0';
    int delNode,delDis=10000;

    for (int ii = 0; ii < 10; ii++){

        for (int i = 0; i < 10; ++i)
            for (int j = 0; j < 10; ++j)
                cost[i][j] = cons[i][j];

        if((cost[s][ii]>=10000)||(cost[s][ii]==0)){
            continue;
        }

        delNode=ii;
        delDis=cost[s][ii];
        cost[s][delNode]=10000;
        cost[delNode][s]=10000;


        int dStart=delNode;
        int djs[10][3]={0};
        for (int i = 0; i < 10; i++){ 
            djs[i][1]=10000;
            djs[i][2]=10000;
        }
        djs[dStart][1]=0; 
        djs[dStart][2]=dStart;

        for (int z = 0; z < 10; z++){ 
            int minValue=10000,node=0;

            for (int i = 0; i < 10; i++){
                if((djs[i][1]<minValue)&&(djs[i][0]==0)){
                    node=i;
                    minValue=djs[i][1];
                }
            }
            
            djs[node][0]=1; 
            
            for(int i = 0;i < 10; i++){
                if((cost[node][i]<10000)&&(cost[node][i]!=0)){
                    if(cost[node][i]+djs[node][1]<djs[i][1]){ 
                        djs[i][1]=cost[node][i]+djs[node][1];
                        djs[i][2]=node; 
                    } 
                }
            }

        }
        if (djs[s][1] >= 10000) {
            continue;
        }

        intp[0]=s;
        int q=s; 
        j=0;
        while (1){
            j++;
            intp[j]=djs[q][2];
            if(q==intp[j])
                break;
            q=intp[j];
        }

        int leng=10000;
        leng=djs[s][1]+delDis;
        if(bestl>leng){
            oj=j;
            bestl=leng;
            for (int i = 0; i < 10; ++i)
                ans[i] = intp[i];
        }
    }

    if(bestl==10000){
        //p[0]='n';
        strcpy(p, "There is no path");
        return 0;
    }

    char buffer[10];
    sprintf(buffer, "%d", ans[0]);
    strcat(p, buffer);
    for(int i = oj-1; i>=0; i--){
        sprintf(buffer, ",%d", ans[i]); 
        strcat(p, buffer); 
    }
    return 0;
}
//---------------------
int main(int argc, char **argv) {
    struct sockaddr_in server, client;
    int sockfd, connfd;
    int clen;
    char buf[100];
    //------create graph------
    int cost[10][10];
    int n,i,j,s,d;
    char p[100];
    for(i=0;i<10;i++)
    {
        for(j=0;j<10;j++)
        { 
           cost[i][j]=10000;
        }  
    }
    cost[0][0]=0;
    cost[0][1]=1;
    cost[0][3]=4;
    cost[1][0]=1;
    cost[1][1]=0;
    cost[1][2]=3;
    cost[1][4]=1;
    cost[2][1]=3;
    cost[2][2]=0;
    cost[2][4]=1;
    cost[2][5]=2;
    cost[3][0]=4;
    cost[3][3]=0;
    cost[3][4]=1;
    cost[4][1]=1;
    cost[4][2]=1;
    cost[4][3]=1;
    cost[4][4]=0;
    cost[5][2]=2;
    cost[5][5]=0;

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  //INADDR_ANY: bind all local interfaces. 
    server.sin_port = htons(8888);


    //Create socket
    sockfd = socket(AF_INET, SOCK_STREAM , 0);
    if(sockfd == -1)
    {
        printf("Error: socket\n");
	    return 0;
    }
     
     
    //Bind
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("Error: bind\n");
        return 0;
    } 
     
    //Listen
    if(listen(sockfd , 3)==-1){
	    printf("Error listen\n");
	    return 0;
    }
 
        //Accept connection from an incoming client
    clen = sizeof(client);
    connfd = accept(sockfd, (struct sockaddr *)&client, &clen);
    if (connfd==-1){
        printf("Error: accept\n");
        return 0;
    }
        
    bzero(buf, sizeof(buf));
    read(connfd, buf, sizeof(buf));
    path(cost, atoi(buf), p); // original path function, buf[0] is the source node
    printf("path: %s\n", p);
    write(connfd, p, strlen(p));
    close(connfd);
        
        
    return 0;
}
