#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <omp.h>
#include "timer.h"

typedef struct ball
{
	double px,py,pz; // 坐标
	double vx,vy,vz; // 速度
	double ax,ay,az; // 加速度
}ball;

ball            ball_list[64];
const int       N=64;
const double    GM=6.67E-7;         //G*M
int             timestep=100;
double          delta_t=0.0;
double          r=0.01;             //球的半径
int             cycle_times=20000;  //周期数
int             size=0;             //方阵宽
FILE*           fp;

char* filename[4]={"result1.txt","result2.txt","result3.txt","result4.txt"};

void compute_force(int index)
{
	ball_list[index].ax=0;
	ball_list[index].ay=0;
	ball_list[index].az=0;
	for(int i=0;i<N;i++)
	{
		if(i!=index)
		{
			double dx=ball_list[i].px-ball_list[index].px;
			double dy=ball_list[i].py-ball_list[index].py;
			double dz=ball_list[i].pz-ball_list[index].pz;
			double d=(dx*dx+dy*dy+dz*dz);
			if(d<r*r)d=r*r;
			d*=sqrt(d);//^(3/2)
			ball_list[index].ax+=GM*(dx)/d;
			ball_list[index].ay+=GM*(dy)/d;
			ball_list[index].az+=GM*(dz)/d;
			//printf("%lf %lf  ",dx,dy);
		}
	}
	//printf("%d a: %lf %lf %1f\n",index,ball_list[index].ax,ball_list[index].ay,ball_list[index].az);
}

void compute_velocities(int index)
{
	ball_list[index].vx+=ball_list[index].ax*delta_t;
	ball_list[index].vy+=ball_list[index].ay*delta_t;
	ball_list[index].vz+=ball_list[index].az*delta_t;
	//printf("%d v: %lf %lf\n",index,ball_list[index].vx,ball_list[index].vy);
}

void compute_positions(int index)
{
	ball_list[index].px+=ball_list[index].vx*delta_t;
	if(ball_list[index].px>((size-1)/100.0))ball_list[index].px=(size-1)/100.0;
	if(ball_list[index].px<0)ball_list[index].px=0;
	ball_list[index].py+=ball_list[index].vy*delta_t;
	if(ball_list[index].py>((size-1)/100.0))ball_list[index].py=(size-1)/100.0;
	if(ball_list[index].py<0)ball_list[index].py=0;
	ball_list[index].pz+=ball_list[index].vz*delta_t;
	if(ball_list[index].pz>((size-1)/100.0))ball_list[index].pz=(size-1)/100.0;
	if(ball_list[index].pz<0)ball_list[index].pz=0;
	//printf("%d p: %lf %lf %1f\n",index,ball_list[index].px,ball_list[index].py,ball_list[index].pz);
}

void print()
{
	//int table[16][16]={0};
	for(int i=0;i<N;i++)
	{
		//table[(int)(ball_list[i].px*100)][(int)(ball_list[i].py*100)]++;
		fprintf(fp,"%lf\n",ball_list[i].px);
	}
	fprintf(fp,"\n");
	for(int i=0;i<N;i++)
	{
		//table[(int)(ball_list[i].px*100)][(int)(ball_list[i].py*100)]++;
		fprintf(fp,"%lf\n",ball_list[i].py);
	}
	fprintf(fp,"\n");
	for(int i=0;i<N;i++)
	{
		//table[(int)(ball_list[i].px*100)][(int)(ball_list[i].py*100)]++;
		fprintf(fp,"%lf\n",ball_list[i].pz);
	}
	fprintf(fp, "end of printing\n\n");
}

int main(int argc,char *argv[])
{
	//init
	delta_t=1.0/timestep;
	//printf("%lf\n",delta_t);
	size=ceil(pow(N,1.0/3));
	//printf("%d\n",size);

	for(int i=0;i<N;i++)
	{
		ball_list[i].px=0.01*(i%size);
		ball_list[i].py=0.01*(i/(size*size));
		ball_list[i].pz=0.01*((i/size)%size);
		ball_list[i].vx=0;
		ball_list[i].vy=0;
		ball_list[i].vz=0;
		ball_list[i].ax=0;
		ball_list[i].ay=0;
		ball_list[i].az=0;
	}
    double starttime,endtime;
	//模拟开始
	// starttime=clock();
	GET_TIME(starttime);
    fp = fopen(filename[0],"w");
	for(int i=0;i<cycle_times;i++)
	{
		for(int j = 0;j < N;j++)
		{
			compute_force(j);
		}
		for(int j = 0;j < N;j++)
		{
			compute_velocities(j);
			compute_positions(j);
		}
		//printf("\n");
	}
	print();
	fclose(fp);
	// endtime=clock();
	GET_TIME(endtime);
	printf("serial 3-dimension time:%lf\n", endtime-starttime);
    return 0;
}
