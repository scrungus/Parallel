#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */


pthread_barrier_t passa,passb;

typedef struct cell {
    int x;
    int y;
    double prev;
    double curr;
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

CELL *new_cell(int x, int y, double prev, double curr){
    CELL *pa = malloc(sizeof(CELL));
    if(pa==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    pa->x = x;
    pa->y = y;
    pa->prev = prev;
    pa->curr = curr;
    return pa;
}

int check_done(CELL *aa, double P){
    int done = 0;
    while(aa != NULL){
        if(fabs(aa->curr - aa->prev)<=P){
            done = 1;
        }
        else{
            done = 0;
            break;
        }
        aa=aa->next;
    }
    return done;
}

void printdiff(CELL *a, double P){
    int count = 0;
    printf("================================================================\n");
    while(a != NULL){
        if(fabs(a->curr - a->prev)<=P){printf("(%.3f,%.3f) ",a->prev,a->curr);}
        else{printf("(" RED" %.3f" RESET","RED"%.3f"RESET") ",a->prev,a->curr);}
        
        a = a->next;
        count++;
        if(count ==4){count = 0; printf("\n");}
    }
    printf("================================================================\n");
}
void compute0(double **a, CELL *pa, CELL *pb, int id, int P){
    double avg,a1,a2,a3,a4;
    CELL *aa,*bb;
    int count=0;
    //First pass computation
    while(1){
        count++;
        pthread_barrier_wait(&passa);
        aa = pa->next;
        bb = pb->next;
        if(check_done(aa,P) && check_done(bb,P)){
            printf("================================================================\n");
            printf("DONE in %d passes\n",count);
             printf("================================================================\n");
             printf("PASS A\n");
            printdiff(aa,P);
            printf("PASS B\n");
            printdiff(bb,P);
            exit(1);
        }
        else{
            printf("PASS A\n");
            printdiff(aa,P);
            printf("PASS B\n");
            printdiff(bb,P);
        }
        while(aa != NULL){
            a1 = a[aa->x-1][aa->y];
            a2 = a[aa->x][aa->y-1];
            a3 = a[aa->x+1][aa->y];
            a4 = a[aa->x][aa->y+1];
            avg = (a1+a2+a3+a4)/4;
            a[aa->x][aa->y] = avg;
            aa->prev = aa->curr;
            aa->curr = avg;
            aa = aa->next;
        }
        printa(a,10);
        printf("================================================================\n");
        //second pass
        pthread_barrier_wait(&passb);
        while(bb != NULL){
            avg = (a[bb->x-1][bb->y] + 
            a[bb->x][bb->y-1] + 
            a[bb->x+1][bb->y] + 
            a[bb->x][bb->y+1])/4;

            a[bb->x][bb->y] = avg;
            bb->prev = bb->curr;
            bb->curr = avg;
            bb = bb->next;
        }
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

    pthread_barrier_init(&passa,NULL,1);
    pthread_barrier_init(&passb,NULL,1);

    CELL *pa = malloc(sizeof(CELL));
    CELL *aa = pa;
    CELL *pb = malloc(sizeof(CELL));
    CELL *bb = pb;
    if(pa==NULL||aa==NULL||bb==NULL||pb==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}

    int k = 0, count = 0;

    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            count++;
            pa->next = new_cell(i,j,0,a[i][j]);
            pa=pa->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    k=1;
    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            count++;
            pb->next = new_cell(i,j,0,a[i][j]);
            pb=pb->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    compute0(a,aa,bb,1,P);

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
        //printa(a,n);
    }
}
