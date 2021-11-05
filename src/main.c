#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */


pthread_barrier_t passa,passb,check;

typedef struct thread_status {
    unsigned long id;
    int status;
    struct thread_status *next;
    int count;
}TSTAT;

TSTAT *tstat;

typedef struct cell {
    int x;
    int y;
    double prev[4];
    double curr;
    double prevv;
    struct cell *next;
}CELL;

typedef struct argt {
    double **a;
    CELL *pa;
    CELL *pb;
    double P;
}ARGT;

void printa(double **a, int n){
    for(int i = 0; i < n;i++){
            for(int j = 0; j < n; j++){
                printf("%.1f ", a[i][j]);
            }
            printf("\n");
        }
}

TSTAT *new_tstat(pthread_t thread){
    TSTAT *tstat = malloc(sizeof(TSTAT));
    tstat->id = thread;
    tstat->status = 0;
    tstat->count = 0;
    return tstat;
}

void update_tstat(pthread_t thread, int count){
    TSTAT *threadid = tstat;
    while(threadid != NULL){
        if(threadid->id == thread && threadid->status == 0 ){
            threadid->status = 1;
            threadid->count= count;
            break;
        }
        threadid = threadid->next;
    }
}

TSTAT *find_tstat(pthread_t thread){
    TSTAT *threadid = tstat;
    while(threadid != NULL){
        if(threadid->id == thread){
            return threadid;
        }
        threadid = threadid->next;
    }
    return NULL;
}

ARGT *new_args(double **a, CELL *pa, CELL *pb, double P){
    ARGT *argt = malloc(sizeof(ARGT));
    if(argt==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    argt->a = a;
    argt->pb = pb;
    argt->pa = pa;
    argt->P = P;

    return argt;
}

CELL *new_workt(CELL *a, int n){
    CELL *workt = malloc(sizeof(CELL));
    if(workt==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    CELL *first = workt;

    while(n-- && a != NULL){
        workt->next = malloc(sizeof(CELL));
        *workt->next = *a;
        workt = workt->next;
        a = a->next;
    }
    workt->next = NULL;
    return first;
}

CELL *new_cell(int x, int y){
    CELL *pa = malloc(sizeof(CELL));
    if(pa==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    pa->x = x;
    pa->y = y;
    for (int i = 0; i < 4;i++){
        pa->prev[i] = 0;
    }
    pa->prevv = 0;
    return pa;
}

int check_done(CELL *aa, double P, double** a){
    int done = 0;
    while(aa != NULL){
        if(fabs(aa->curr - aa->prevv)<=P){
            int a1 = fabs(aa->prev[0] - a[aa->x-1][aa->y]);
            int a2 = fabs(aa->prev[1] - a[aa->x][aa->y-1]);
            int a3 = fabs(aa->prev[2] - a[aa->x+1][aa->y]);
            int a4 = fabs(aa->prev[3] - a[aa->x][aa->y+1]);
            if((a1+a2+a3+a4)<= P){done =1;}
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
        if(fabs(a->curr - a->prevv)<=P){printf("(%.3f,%.3f) ",a->prevv,a->curr);}
        else{printf("(" RED" %.3f" RESET","RED"%.3f"RESET") ",a->prevv,a->curr);}
        
        a = a->next;
        count++;
        if(count ==4 || a==NULL){count = 0; printf("\n");}
    }
    printf("================================================================\n");
}
void *compute0(void *argsv){
    double avg,P,**a;
    CELL *aa,*bb, *pa, *pb;
    int count=0;
    ARGT *args = (ARGT*)argsv;
    a = args->a;
    pa = args->pa;
    pb = args->pb;
    P = args->P;
    int done;
    //First pass computation
    while(1){
        pthread_barrier_wait(&check);
        count++;
        aa = pa->next;
        done = 0;
        if(check_done(aa,P,a)){
            done = 1;
        }
        pthread_barrier_wait(&passa);
          // printf("Thread %lu Barrier A\n",pthread_self());
           fflush( stdout );
        while(aa != NULL){
            avg = (a[aa->x-1][aa->y] + 
            a[aa->x][aa->y-1] + 
            a[aa->x+1][aa->y] + 
            a[aa->x][aa->y+1])/4;

            a[aa->x][aa->y] = avg;
            aa->prevv = aa->curr;
            aa->curr = avg;
            aa->prev[0] = a[aa->x-1][aa->y];
            aa->prev[1] =a[aa->x][aa->y-1];
            aa->prev[2] = a[aa->x+1][aa->y];
            aa->prev[3] =a[aa->x][aa->y+1];
            aa = aa->next;
        }
        //second pass
        pthread_barrier_wait(&passb);
            //printf("Thread %lu Barrier B\n",pthread_self());
           fflush( stdout );
        while(bb != NULL){
            avg = (a[bb->x-1][bb->y] + 
            a[bb->x][bb->y-1] + 
            a[bb->x+1][bb->y] + 
            a[bb->x][bb->y+1])/4;

            a[bb->x][bb->y] = avg;
            bb->prevv = bb->curr;
            bb->curr = avg;
            bb->prev[0] = a[bb->x-1][bb->y];
            bb->prev[1] =a[bb->x][bb->y-1];
            bb->prev[2] = a[bb->x+1][bb->y];
            bb->prev[3] =a[bb->x][bb->y+1];
            bb = bb->next;
        }
        bb = pb->next;
        if(done && check_done(bb,P,a)){
            update_tstat(pthread_self(),count);
        }
    }
}

void compute1(void *argsv){
    double avg,P,**a;
    CELL *aa,*bb, *pa, *pb;
    int count=0;
    ARGT *args = (ARGT*)argsv;
    a = args->a;
    pa = args->pa;
    pb = args->pb;
    P = args->P;
    //First pass computation
    while(1){
        count++;
        aa = pa->next;
        bb = pb->next;
        if(check_done(aa,P,a) && check_done(bb,P,a)){
            printf("completed in %d passes\n",count);
            printf("================================================================\n");  
            return;
        }
        while(aa != NULL){
            avg = (a[aa->x-1][aa->y] + 
            a[aa->x][aa->y-1] + 
            a[aa->x+1][aa->y] + 
            a[aa->x][aa->y+1])/4;

            a[aa->x][aa->y] = avg;
            aa->prevv = aa->curr;
            aa->curr = avg;
            aa->prev[0] = a[aa->x-1][aa->y];
            aa->prev[1] =a[aa->x][aa->y-1];
            aa->prev[2] = a[aa->x+1][aa->y];
            aa->prev[3] =a[aa->x][aa->y+1];
            aa = aa->next;
        }
        while(bb != NULL){
            avg = (a[bb->x-1][bb->y] + 
            a[bb->x][bb->y-1] + 
            a[bb->x+1][bb->y] + 
            a[bb->x][bb->y+1])/4;

            a[bb->x][bb->y] = avg;
            bb->prevv = bb->curr;
            bb->curr = avg;
            bb->prev[0] = a[bb->x-1][bb->y];
            bb->prev[1] =a[bb->x][bb->y-1];
            bb->prev[2] = a[bb->x+1][bb->y];
            bb->prev[3] =a[bb->x][bb->y+1];
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

CELL *find_next(CELL *a , int n){
    while(n-- && a != NULL){
        a = a->next;
    }
    return a;
}

double **compute (int p, double P, int n, double **a){

    p--;
    pthread_barrier_init(&passa,NULL,p);
    pthread_barrier_init(&passb,NULL,p);
    pthread_barrier_init(&check,NULL,p);
    pthread_t *threads = malloc(sizeof(pthread_t)*p);
    tstat = malloc(sizeof(TSTAT));
    TSTAT *tstat0 = tstat;

    CELL *pa = malloc(sizeof(CELL));
    CELL *aa = pa;
    CELL *pb = malloc(sizeof(CELL));
    CELL *bb = pb;
    if(pa==NULL||aa==NULL||bb==NULL||pb==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}

    int k = 0, counta = 0, countb = 0;

    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            counta++;
            pa->next = new_cell(i,j);
            pa=pa->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    k=1;
    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            countb++;
            pb->next = new_cell(i,j);
            pb=pb->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    ARGT *args;

    if(p==0){
        printf("1 thread provided, running sequentially...\n");
        printf("================================================================\n");  
        args = new_args(a,aa,bb,P);
        compute1(args);
        return a;
        
    }
    printf("================================\n");
    printf("note: one thread is used for control, computations will be run on %d threads\n",p);

    int worka = counta/p, workb = countb/p;
    int extraa = counta%p, extrab = countb%p;
    CELL *workta;
    CELL *worktb;
    

    int i =0;
    while(i<p){
        if(extraa){
            workta = new_workt(aa->next,worka+1);
            extraa--;
            aa = find_next(aa, worka+1);
        }
        else{
             workta = new_workt(aa->next,worka);
             aa = find_next(aa, worka);
        }
        if(extrab){
            worktb = new_workt(bb->next,workb+1);
            extrab--;
            bb = find_next(bb, workb+1);
        }
        else{
             worktb = new_workt(bb->next,workb);
             bb = find_next(bb, workb);
        }
        args = new_args(a,workta,worktb,P);
        int err = pthread_create(&threads[i],NULL,&compute0,args);
        if(err){
            printf("Thread Creation Error, exiting..\n");
        }
        else{
            printf("Thread Created with ID : %lu\n",threads[i]);
            fflush( stdout );
            tstat0->next = new_tstat(threads[i]);
            tstat0 = tstat0->next;
        }
        i++;
    }
    printf("================================================================\n");   
    int done=0;
    while(!done){
        tstat0 = tstat->next;
        done = 1;
        while(tstat0 != NULL){
            if(tstat0->status == 0){done = 0;break;}
            tstat0 = tstat0->next;
        }
    }
    for(int i=0;i<p; i++){
        printf("Thread %lu completed in %d passes\n",threads[i],find_tstat(threads[i])->count);
        pthread_cancel(threads[i]);
    }
    printf("================================================================\n");   
    return a;
}

double **readfromfile(char *filename, int n){
    FILE *f = fopen(filename,"r");
    double **a;
    if (f == NULL){printf("fatal: no such file or invalid format : %s\n",filename);exit(1);}
    char *c = malloc(sizeof(char*));
    int x = 0; int y = 0;
    a = malloc(n * sizeof(*a));
    for(int i=0; i<n; i++){
        a[i] = malloc(n * sizeof(*a[i]));
    }
    while(!feof(f)){
        fscanf(f,"%s",c);
        a[x][y] = strtod(c,NULL);
        y++;
        if (y == n){
            x++;
            y=0;
        }
        if(x == n){
            break;
        }        
    }
    fclose(f);
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
                    if((*argv)[2] == 'f'){
                        a = readfromfile((*++argv),n);
                        break;
                    }
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
        //printa(a,n);
        printf("================================\n");
        printf("COMPUTED: \n");
        printf("================================\n");
        a = compute(p,P,n,a);
        //printa(a,n);
    }
}
