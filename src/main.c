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
    int id;
    int x;
    int y;
    double prev[4];
    double curr;
    double prevv;
    struct cell *next;
}CELL;

typedef struct argt {
    double **a;
    int starta;
    int worka;
    int startb;
    int workb;
    double P;
}ARGT;

CELL *aa, *bb;

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

ARGT *new_args(double **a,int starta, int worka, int startb, int workb, double P){
    ARGT *argt = malloc(sizeof(ARGT));
    if(argt==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    argt->a = a;
    argt->starta = starta;
    argt->worka = worka;
    argt->startb = startb;
    argt->workb = workb;
    argt->P = P;

    return argt;
}

CELL *find_next(CELL *a , int id){
    CELL *p = a;
    while(p != NULL){
        if(p->id == id){
            return p;
        }
        p = p->next;
    }
    return NULL;
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

CELL *new_workt(int start, int work,CELL *pa,pthread_t t, char c){
    CELL *workt = malloc(sizeof(CELL));
    if(workt==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    CELL *first = workt;
    CELL *a = find_next(pa,start);
    pthread_mutex_lock(&mutex);
    printf("Thread %lu given work: ",t);
    while(work-- && a != NULL){
        workt->next = malloc(sizeof(CELL));
        *workt->next = *a;
        printf("(%d,%d) ",a->x,a->y);
        workt = workt->next;
        a = a->next;
    }
    printf("for Pass %c \n",c);
    fflush( stdout );
    pthread_mutex_unlock(&mutex);
    workt->next = NULL;
    return first;
}

CELL *new_cell(int x, int y, int id){
    CELL *pa = malloc(sizeof(CELL));
    if(pa==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}
    pa->id = id;
    pa->x = x;
    pa->y = y;
    for (int i = 0; i < 4;i++){
        pa->prev[i] = 0;
    }
    pa->prevv = 0;
    return pa;
}

int check_done(CELL *pa, double P, double** a){
    int done = 0;
    CELL *p = pa;
    while(p != NULL){
        if(fabs(pa->curr - pa->prevv)<=P){
            int a1 = fabs(p->prev[0] - a[p->x-1][p->y]);
            int a2 = fabs(p->prev[1] - a[p->x][p->y-1]);
            int a3 = fabs(p->prev[2] - a[p->x+1][p->y]);
            int a4 = fabs(p->prev[3] - a[p->x][p->y+1]);
            if((a1+a2+a3+a4)<= P*4){done =1;}
        }
        else{
            done = 0;
            break;
        }
        p=p->next;
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
    CELL *pa, *pb, *p1, *p2;
    int count=0;
    ARGT *args = (ARGT*)argsv;
    a = args->a;
    P = args->P;
    p1 = new_workt(args->starta,args->worka,aa,pthread_self(),'A');
    p2 = new_workt(args->startb,args->workb,bb,pthread_self(),'B');
    pa = p1->next;
    pb = p2->next;
    printf("Thread %lu has starta %d, worka %d, startb %d, workb %d\n",pthread_self(),args->starta,args->worka,args->startb,args->workb);
    int done,complete =0;
    //First pass computation
    while(1){
        count++;
        pthread_barrier_wait(&passa);
          // printf("Thread %lu Barrier A\n",pthread_self());
           fflush( stdout );
        while((!complete) && pa != NULL){
            avg = (a[pa->x-1][pa->y] + 
            a[pa->x][pa->y-1] + 
            a[pa->x+1][pa->y] + 
            a[pa->x][pa->y+1])/4;

            a[pa->x][pa->y] = avg;
            pa->prevv = pa->curr;
            pa->curr = avg;
            pa->prev[0] = a[pa->x-1][pa->y];
            pa->prev[1] =a[pa->x][pa->y-1];
            pa->prev[2] = a[pa->x+1][pa->y];
            pa->prev[3] =a[pa->x][pa->y+1];
            pa = pa->next;
        }
        pthread_barrier_wait(&check);
            pb = p2->next;
            done = 0;
            if(check_done(pb,P,a)){
                done = 1;
            }
            else{
                //printf("Thread %lu Pass %d, check on B failed\n",pthread_self(),count);
                fflush( stdout );
            }
        //second pass
        pthread_barrier_wait(&passb);
            //printf("Thread %lu Barrier B\n",pthread_self());
           fflush( stdout );
        while((!complete) && pb != NULL){
            avg = (a[pb->x-1][pb->y] + 
            a[pb->x][pb->y-1] + 
            a[pb->x+1][pb->y] + 
            a[pb->x][pb->y+1])/4;

            a[pb->x][pb->y] = avg;
            pb->prevv = pb->curr;
            pb->curr = avg;
            pb->prev[0] = a[pb->x-1][pb->y];
            pb->prev[1] =a[pb->x][pb->y-1];
            pb->prev[2] = a[pb->x+1][pb->y];
            pb->prev[3] =a[pb->x][pb->y+1];
            pb = pb->next;
        }
        pthread_barrier_wait(&check);
        pa = p1->next;
        if(done && check_done(pa,P,a)){
            update_tstat(pthread_self(),count);
        }
        //else{printf("Thread %lu Pass %d, check on A failed\n",pthread_self(),count);fflush( stdout );}
    }
}

void compute1(double **a, double P){
    double avg;
    int count=0;
    CELL *pa = aa->next;
    CELL *pb = bb->next;
    //First pass computation
    int done;
    while(1){
        count++;
        done = 0;
        while(pa != NULL){
            avg = (a[pa->x-1][pa->y] + 
            a[pa->x][pa->y-1] + 
            a[pa->x+1][pa->y] + 
            a[pa->x][pa->y+1])/4;

            a[pa->x][pa->y] = avg;
            pa->prevv = pa->curr;
            pa->curr = avg;
            pa->prev[0] = a[pa->x-1][pa->y];
            pa->prev[1] =a[pa->x][pa->y-1];
            pa->prev[2] = a[pa->x+1][pa->y];
            pa->prev[3] =a[pa->x][pa->y+1];
            pa = pa->next;
        }
        pb = bb->next;
        if(check_done(pb,P,a)|| pb == NULL){
            done = 1;
        }
        while(pb != NULL){
            avg = (a[pb->x-1][pb->y] + 
            a[pb->x][pb->y-1] + 
            a[pb->x+1][pb->y] + 
            a[pb->x][pb->y+1])/4;

            a[pb->x][pb->y] = avg;
            pb->prevv = pb->curr;
            pb->curr = avg;
            pb->prev[0] = a[pb->x-1][pb->y];
            pb->prev[1] =a[pb->x][pb->y-1];
            pb->prev[2] = a[pb->x+1][pb->y];
            pb->prev[3] =a[pb->x][pb->y+1];
            pb = pb->next;
        }
        pa = aa->next;
        if(done && check_done(pa,P,a)){
            printf("completed in %d passes\n",count);
            printf("================================================================\n");  
            return;
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

    p--;
    pthread_barrier_init(&passa,NULL,p);
    pthread_barrier_init(&passb,NULL,p);
    pthread_barrier_init(&check,NULL,p);
    pthread_t *threads = malloc(sizeof(pthread_t)*p);
    tstat = malloc(sizeof(TSTAT));
    TSTAT *tstat0 = tstat;

    CELL *pa = malloc(sizeof(CELL));
    aa = pa;
    CELL *pb = malloc(sizeof(CELL));
    bb = pb;

    if(pa==NULL||aa==NULL||bb==NULL||pb==NULL){printf("Memory allocation failed, exiting.\n");exit(1);}

    int k = 0, counta = 0, countb = 0;

    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            counta++;
            pa->next = new_cell(i,j,counta);
            pa=pa->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    k=1;
    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            countb++;
            pb->next = new_cell(i,j,countb);
            pb=pb->next;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    ARGT *args;

    if(p==0){
        printf("1 thread provided, running sequentially...\n");
        printf("================================================================\n");  
        //args = new_args(a,P);
        compute1(a,P);
        return a;
        
    }
    if(bb->next == NULL){
        printf("Array too small, running sequentially...\n");
        //args = new_args(a,P);
        compute1(a,P);
        return a;
    }
    printf("================================\n");
    printf("note: one thread is used for control, computations will be run on %d threads\n",p);

    int worka = counta/p, workb = countb/p;
    int extraa = counta%p, extrab = countb%p;
    

    int i =0;
    int starta = 1, startb = 1;
    int extra1,extra2; 
    while(i<p){
        extra1 = extra2 = 0;
        if(extraa){
            extraa--;
            extra1=1;

        }
        if(extrab){
            extrab--;
            extra2 = 1;
        }
        args = new_args(a,starta,worka+extra1,startb,workb+extra2,P);
        int err = pthread_create(&threads[i],NULL,&compute0,args);
        if(err){
            printf("Thread Creation Error, exiting..\n");
        }
        else{
            tstat0->next = new_tstat(threads[i]);
            tstat0 = tstat0->next;
        }
        i++;
        starta = starta+worka+extra1;
        startb = startb+workb+extra2;
    }
    //printf("================================================================\n");   
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
        pthread_t t = threads[i];
        int c = find_tstat(threads[i])->count;
        printf("Thread %lu completed in %d passes\n",t,c);
        pthread_cancel(threads[i]);
    }
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
        printf("================================\n");
        printf("COMPUTED: \n");
        printf("================================\n");
        a = compute(p,P,n,a);
        printa(a,n);
    }
}
