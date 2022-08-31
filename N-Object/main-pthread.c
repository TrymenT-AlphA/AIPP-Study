#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

typedef struct ball{
    double px,py,pz;
    double vx,vy,vz;
    double ax,ay,az;
}ball;

typedef struct acceleration{
    double ax,ay,az;
}acceleration;

ball*               ball_list;
const int           N=1024;
const double        GM=6.67E-7;
int                 timestep=100;
double              delta_t=0.0;
double              r=0.01;
int                 cycle_times=20000;
int                 size=0;
FILE*               resule_fp;
FILE*               log_fp;
int                 thread_count;
acceleration**      a_mat;
pthread_barrier_t   barrier;
char*               resulefile;
char*               logfile;

void print(void);
void initialize(void);
void destroy(void);
void* thread_work(void* rank);

int main(int argc,char *argv[]){
    if (argc <= 1){
        fprintf(stderr, "[main] > Usage: %s <thread_count>\n", argv[0]);
        return -1;
    }
    thread_count = strtol(argv[1], NULL, 10);

    pthread_t* thread_handles = malloc(thread_count*sizeof(pthread_t));
    initialize();

    double starttime,endtime;
    clock_t cpu_starttime, cpu_endtime;
    GET_TIME(starttime);
    cpu_starttime = clock();
    fprintf(log_fp, "[main] > simulating...\n");
    fflush(log_fp);
    //模拟开始
    for (long thread = 0; thread < thread_count; thread++){
        pthread_create(&thread_handles[thread], NULL, thread_work, (void*)thread);
    }
    for (long thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);        
    }
    //模拟结束
    cpu_endtime = clock();
    GET_TIME(endtime);

    print();
    double wall_time = endtime-starttime;
    double cpu_time = (double)(cpu_endtime-cpu_starttime)/CLOCKS_PER_SEC;
    double cpu_usage = cpu_time/wall_time*100.0;
    fprintf(log_fp, "[main] > =====summary=====\n");
    fprintf(log_fp, "wall time\t%lf s\n", wall_time);
    fprintf(log_fp, "cup  time\t%lf s\n", cpu_time);
    fprintf(log_fp, "cpu  usage\t%lf %%\n", cpu_usage);
    fprintf(log_fp, "[main] > destroying...\n");
    fflush(log_fp);
    destroy();
    free(thread_handles);
    return 0;
}

void* thread_work(void* rank){
    long my_rank   = (long)rank;
    int delta      = (N+thread_count-1)/thread_count;
    int my_N_start = my_rank*delta;
    int my_N_end   = my_N_start+delta;
    if (my_N_end > N) my_N_end = N;

    fprintf(log_fp, "[%4ld] > From [%4d] To [%4d]\n", my_rank, my_N_start, my_N_end);

    for (int cycle = 0; cycle < cycle_times; ++cycle)
    {
        for (int i = my_N_start; i < my_N_end; i++)
            for (int j = 0; j < N; j++) if (j > i){
                double dx=ball_list[j].px-ball_list[i].px;
                double dy=ball_list[j].py-ball_list[i].py;
                double dz=ball_list[j].pz-ball_list[i].pz;
                double d=(dx*dx+dy*dy+dz*dz);
                if(d<r*r)d=r*r;
                d*=sqrt(d);//^(3/2)
                a_mat[i][j].ax = GM*(dx)/d;
                a_mat[i][j].ay = GM*(dy)/d;
                a_mat[i][j].az = GM*(dz)/d;
            }

        pthread_barrier_wait(&barrier);

        for (int i = my_N_start; i < my_N_end; i++){
            ball_list[i].ax = 0;
            ball_list[i].ay = 0;
            ball_list[i].az = 0;
            for (int j = 0; j < N; j++){
                if (j > i){
                    ball_list[i].ax += a_mat[i][j].ax;
                    ball_list[i].ay += a_mat[i][j].ay;
                    ball_list[i].az += a_mat[i][j].az;
                }
                else if (j < i){
                    ball_list[i].ax -= a_mat[j][i].ax;
                    ball_list[i].ay -= a_mat[j][i].ay;
                    ball_list[i].az -= a_mat[j][i].az;
                }
            }
            ball_list[i].vx+=ball_list[i].ax*delta_t;
            ball_list[i].vy+=ball_list[i].ay*delta_t;
            ball_list[i].vz+=ball_list[i].az*delta_t;
            ball_list[i].px+=ball_list[i].vx*delta_t;
            if(ball_list[i].px>((size-1)/100.0)) ball_list[i].px=(size-1)/100.0;
            if(ball_list[i].px<0) ball_list[i].px=0;
            ball_list[i].py+=ball_list[i].vy*delta_t;
            if(ball_list[i].py>((size-1)/100.0)) ball_list[i].py=(size-1)/100.0;
            if(ball_list[i].py<0) ball_list[i].py=0;
            ball_list[i].pz+=ball_list[i].vz*delta_t;
            if(ball_list[i].pz>((size-1)/100.0)) ball_list[i].pz=(size-1)/100.0;
            if(ball_list[i].pz<0) ball_list[i].pz=0;
        }
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

void print(void){
    for(int i=0;i<N;i++)
        fprintf(resule_fp,"%lf\n",ball_list[i].px);
    fprintf(resule_fp,"\n");
    for(int i=0;i<N;i++)
        fprintf(resule_fp,"%lf\n",ball_list[i].py);
    fprintf(resule_fp,"\n");
    for(int i=0;i<N;i++)
        fprintf(resule_fp,"%lf\n",ball_list[i].pz);
    fprintf(resule_fp, "end of printing\n\n");
}

void initialize(void){
    delta_t=1.0/timestep;
    size=ceil(pow(N,1.0/3));
    resulefile = (char*)malloc(sizeof(char)*128);
    logfile = (char*)malloc(sizeof(char)*128);
    sprintf(resulefile, "result-pthread[%d].txt", thread_count);
    sprintf(logfile, "main-pthread[%d].log", thread_count);
    a_mat = (acceleration**)malloc(sizeof(acceleration*)*N);
    for (int i = 0; i < N; i++)
        a_mat[i] = (acceleration*)malloc(sizeof(acceleration)*N);
    ball_list = (ball*)malloc(sizeof(ball)*N);
    for(int i=0;i<N;i++){
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
    resule_fp = fopen(resulefile,"w");
    log_fp = fopen(logfile, "w");
    pthread_barrier_init(&barrier, NULL, thread_count);
}

void destroy(void){
    free(resulefile);
    free(logfile);
    for (int i = 0; i < N; i++)
        free(a_mat[i]);
    free(a_mat);
    free(ball_list);
    fclose(resule_fp);
    fclose(log_fp);
    pthread_barrier_destroy(&barrier);
}
