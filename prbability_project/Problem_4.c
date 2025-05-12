#include <stdio.h>
#include <stdlib.h>
#include <math.h>
double gen_uniform(double lambda){
	return((double)rand()/((double)RAND_MAX+1.0));
}
double gen_exp(double lambda){
	return (log((1-gen_uniform((double)1))))/(-lambda);
}


int sim(double lambda){
    double F=0;
    double lastArrivalTime=0;
    double serviceOverTime=0;
    double arrivalTime[10001]={0};
    double serviceTime[10000];
    for (int i = 0; i < 10000; i++){
        arrivalTime[i]=gen_exp(lambda);
        serviceTime[i]=gen_exp(1.0);
    }
    serviceOverTime=serviceTime[0];
    for(int i = 0;i < 10000; i++){
        lastArrivalTime+=arrivalTime[i];
        if (lastArrivalTime>(serviceOverTime-serviceTime[i])){
            serviceOverTime=lastArrivalTime+serviceTime[i];
        }else{
            F++;
        }
        printf("%d %lf %lf | %lf %lf -- %lf\n",i,arrivalTime[i],serviceTime[i],lastArrivalTime,serviceOverTime,F-1);
    }
    F=F/10000;
    printf("F= %lf", F);
return 0;
}


int main(){
double lambda;

printf("Input lambda: ");
scanf("%lf", &lambda);
sim(lambda);

return 0;
}
