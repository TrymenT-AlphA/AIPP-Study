/*
    generate_input.c
    Author: ChongKai
*/
#include <stdio.h>
#include <stdlib.h>

#define INIT_NUM 1000
#define OP_NUM 10000

int main(int argc, char* argv[]){
    if (argc <= 1){
        printf("Usage: %s <Member_rate(%%)>\n", argv[0]);
        return 0;
    }
    long Member_rate = strtol(argv[1], NULL, 10)*10;    

    int i, thread, thread_count;
    char filename[32];
    FILE * out;

    thread_count = 8;

    out = fopen("input.txt", "w");
    fprintf(out, "%d\n", INIT_NUM);
    for (i = 0; i < INIT_NUM; i++)
        fprintf(out, "%d\n", rand()%2000);

    for (thread = 0; thread < thread_count; thread++){
        sprintf(filename, "input%d.txt", thread);
        out = fopen(filename, "w");

        fprintf(out, "%d\n", OP_NUM);
        for (i = 0; i < OP_NUM; i++){
            int tmp = rand()%1000;
            if (tmp < Member_rate)
                fprintf(out, "M %d\n", rand()%2000);
            else if (tmp < 500 + Member_rate/2)
                fprintf(out, "I %d\n", rand()%2000);
            else
                fprintf(out, "D %d\n", rand()%2000);
        }

        fflush(out);
        fclose(out);
    }

    return 0;
}
