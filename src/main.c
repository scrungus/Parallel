#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    int p,n;
    double P;
    int **a;

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
                    *++argv;
                    //if((*argv)[0] == '{'){
                        int x = 0; int y = 0;
                        while((*argv)[0] != '\0' && ++(*argv)){
                            if((*argv)[0] == ',' || (*argv)[0] == '}'){

                            }
                            else{
                                a[x][y] = strtol((*argv),NULL,10);
                                y++;
                                if (y == n){
                                    x++;
                                    y=0;
                                }
                                if(x == n){
                                    break;
                                }
                            }
                        }
                    //}
                    break;

            }
        }
    }
    printf("p: %i, P: %.2f, n: %i\n", p, P, n);
    printf("\n");
    for(int i = 0; i < n;i++){
        for(int j = 0; j < n; j++){
            printf("%i ", a[i][j]);
        }
        printf("\n");
    }
}