#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


typedef struct cell {
    int x;
    int y;
    struct cell *next;
}CELL;

void printa(double **a, int n){
    for(int i = 0; i < n;i++){
            for(int j = 0; j < n; j++){
                printf("%.1f ", a[i][j]);
            }
            printf("\n");
        }
}

CELL *new_cell(int x, int y){
    CELL *pa = malloc(sizeof(CELL));
    if(pa==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    pa->x = x;
    pa->y = y;
    return pa;
}

void compute0(double **a, CELL *pa, CELL *pb, int *threads, int id, int p, int P){
    double avg,a1,a2,a3,a4;
    CELL *aa = pa->next;
    CELL *bb = pb;
    int count = 0;
    //First pass computation
    while(aa != NULL){
        a1 = a[aa->x-1][aa->y];
        a2 = a[aa->x][aa->y-1];
        a3 = a[aa->x+1][aa->y];
        a4 = a[aa->x][aa->y+1];
        avg = (a1+a2+a3+a4)/4;
        a[aa->x][aa->y] = avg;
        count++;
        aa = aa->next;
    }
    threads[id] = 1; //set as done
    printa(a,10);
    int free = 0;
    //wait for all other threads
    while(free != 1){
        free = 1;
        for(int i= 0; i < p; i++){
            if(threads[i] != 1){
                free = 0;
                break;
            }
        }
    }
    //second pass
    while(bb != NULL){
        avg = (a[bb->x-1][bb->y] + 
        a[bb->x][bb->y-1] + 
        a[bb->x+1][bb->y] + 
        a[bb->x][bb->y+1])/4;

        a[bb->x][bb->y] = avg;

        bb = bb->next;
    }
}

void printcell(CELL *a){
    printf("printing cell ..\n");
    while(a != NULL){
        printf("(%d,%d)\n",a->x,a->y);
        a = a->next;
    }
}

double **compute (int p, double P, int n, double **a){

    int threads[p];
    memset(threads, 0, sizeof(int)*p);
    CELL *pa = malloc(sizeof(CELL));
    CELL *aa = pa;
    if(pa==NULL||aa==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}

    int k = 0, count = 0;

    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            count++;
            pa->next = new_cell(i,j);
            pa=pa->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    printf("success\n");
    compute0(a,aa,NULL,threads,1,p,P);

    int work = count/p;
    int extra = count%p;

    printf("work = %d, extra = %d\n",work,extra);
    return a;
}

int main(int argc, char **argv){
    int p,n;
    double P;
    double **a;

    while(*++argv){
        if((*argv)[0] == '-'){
            switch ((*argv)[1]) {
                case 'p':
                    p = strtol((*++argv),NULL,10);
                    break;
                case 'n':
                    n = strtol((*++argv),NULL,10);
                    break;
                case 'P':
                    P = strtod((*++argv),NULL);
                    break;
                case 'a':
                    a = malloc(n * sizeof(*a));
                    for(int i=0; i<n; i++){
                        a[i] = malloc(n * sizeof(*a[i]));
                    }
                    int x = 0; int y = 0;
                    while(*++argv){
                        a[x][y] = strtod((*argv),NULL);
                        y++;
                        if (y == n){
                            x++;
                            y=0;
                        }
                        if(x == n){
                            break;
                        }        
                    } 
                    break;

            }
        }
    }
    if(p > 0  && P > 0.0 && n > 0 && a != NULL){
        printf("p: %i, P: %.2f, n: %i\n", p, P, n);
        printf("\n");
        printf("================================\n");
        printf("INITIAL: \n");
        printf("================================\n");
        printa(a,n);
        printf("================================\n");
        printf("COMPUTED: \n");
        printf("================================\n");
        a = compute(p,P,n,a);
        printa(a,n);
    }
}
