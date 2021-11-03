#include <stdio.h>
#include <stdlib.h>


void printa(double **a, int n){
    for(int i = 0; i < n;i++){
            for(int j = 0; j < n; j++){
                printf("%.1f ", a[i][j]);
            }
            printf("\n");
        }
}

double **compute (int p, double P, int n, double **a){
    int k = 0;
    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            a[i][j] = 0.0;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
    k = 1;
    for(int i = 1; i < n-1; i++){
        for(int j = 1+k; j < n-1; j+=2){
            a[i][j] = 1.0;
        }
        if(k ==0){k=1;}
        else if (k ==1){k=0;}
    }
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
