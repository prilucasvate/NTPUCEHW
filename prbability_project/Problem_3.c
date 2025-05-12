#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

double lastArrivalTime=0;
double serviceOverTime=0;
double arrivalTime[10001]={0};
double serviceTime[10000];

double setArrivalTime(int i){
    lastArrivalTime+=arrivalTime[i];
}
double setServiceTime(int i){
    serviceOverTime+=serviceTime[i];
    if(lastArrivalTime>(serviceOverTime-serviceTime[i])){
        serviceOverTime=lastArrivalTime+serviceTime[i];
    }
}

double AvgPackets(double *serviceTime,double *arrivalTime){
    lastArrivalTime=0;
    serviceOverTime=0;
    double max=0;
    double totalN=0;
    int j=0;
    double Ni[10000];
    for (int i = 0; i < 10000; i++){
        setArrivalTime(i);
        setServiceTime(i);
        while (max<serviceOverTime){   
        max+=arrivalTime[++j];
        }
        Ni[i]=j-1-i;
        totalN+=Ni[i];
    }
    return totalN/10000;
}
double AvgTime(double *serviceTime,double *arrivalTime){
    lastArrivalTime=0;
    serviceOverTime=0;
    double totalT=0;
    for(int i = 0;i < 10000; i++){
        setArrivalTime(i);
        setServiceTime(i);
        totalT+=(serviceOverTime-lastArrivalTime);
    }
    return totalT/10000;
}
double gen_uniform(double lambda){
	return((double)rand()/((double)RAND_MAX+1.0));
}
double gen_exp(double lambda){
	return (log((1-gen_uniform((double)1))))/(-lambda);
}


int sim(double lambda){
    double N=0, T=0;
    for (int i = 0; i < 10000; i++){
        arrivalTime[i+1]=gen_exp(lambda);
        serviceTime[i]=gen_exp(1.0);
    }

    N=AvgPackets(serviceTime,arrivalTime);
    T=AvgTime(serviceTime,arrivalTime);
    printf("N= %lf, T= %lf\n", N, T);

return 0;
}


int main(){
double lambda;

printf("Input lambda: ");
scanf("%lf", &lambda);
sim(lambda);

return 0;
}
