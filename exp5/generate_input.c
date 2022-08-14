#include <stdio.h>
#include <stdlib.h>

#define INIT_NUM 1000
#define OP_NUM 10000

int main(int argc, char* argv[]){
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
        for (i =0; i < OP_NUM; i++){
            int tmp = rand()%1000;
            if (tmp < 800)
                fprintf(out, "M %d\n", rand()%2000);
            else if (tmp < 900)
                fprintf(out, "I %d\n", rand()%2000);
            else
                fprintf(out, "D %d\n", rand()%2000);
        }

        fflush(out);
        fclose(out);
    }

    return 0;
}
