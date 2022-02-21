#include <stdio.h>
#include <stdlib.h>
#include "ctimer.h"
#include <assert.h>

typedef struct {
    double *R;
    double *G;
    double *B;
}Img;

void imprimirMat(int, int, double *);
void convolucion(const double*, int, int, const double*, int, int, double*);
int leerMat(int m, int n, int marco, double *mat, const char *filename);

int main( int argc, char *argv[] ){

    if ( argc < 6 ){
        printf("\nUsage: %s mA nA mF nF numImg [padding=0] \n", argv[0]);
        exit(-1);
    }
    int padding = 0;

    /* Padding */
    if ( (argc == 7) && (atoi(argv[6]) == 1) ) {
        padding = 1;
    }

    unsigned int mA, nA, mF, nF, numImg;
    mA = atoi(argv[1]);
    nA = atoi(argv[2]);
    mF = atoi(argv[3]);
    nF = atoi(argv[4]);
    numImg = atoi(argv[5]);

    //Calcular tamaño del marco (si padding esta activado)
    unsigned int marco = 0;
    if (padding) {
        assert(mF == nF); //No funciona con un filtro no cuadrado
        assert(mF % 2 != 0); //No funciona con números pares
        
        marco = (mF-1) / 2;
        mA += marco * 2;
        nA += marco * 2;
    }

    unsigned int sizeA,sizeF;
    sizeA = mA * nA;
    sizeF = mF * nF;

    //Batch de imagenes entrada
    Img batch[numImg];

    //Inicializar el batch
    for (int ind=0; ind < numImg; ind++){
        /* Reserva de memoria */
        batch[ind].R = (double *) calloc(sizeA, sizeof(double)); //R se llena de ceros
        batch[ind].G = (double *) calloc(sizeA, sizeof(double)); //G se llena de ceros
        batch[ind].B = (double *) calloc(sizeA, sizeof(double)); //B se llena de ceros

        /* Generación de la matriz R */
        for (int i=marco; i < mA-marco; i++){
            for (int j=marco; j < nA-marco; j++){
                batch[ind].R[i*nA + j] = drand48();
            }
        }
        /* Generación de la matriz G */
        for (int i=marco; i < mA-marco; i++){
            for (int j=marco; j < nA-marco; j++){
                batch[ind].G[i*nA + j] = drand48();
            }
        }
        /* Generación de la matriz B */
        for (int i=marco; i < mA-marco; i++){
            for (int j=marco; j < nA-marco; j++){
                batch[ind].B[i*nA + j] = drand48();
            }
        }
    }
    /* Reserva de memoria */
    double *F;    
    F = (double *) malloc(sizeof(double) * sizeF);    

    
    /* Generación matriz F */
    for (int i=0; i < mF; i++){
        for (int j=0; j < nF; j++){
            F[i*nF + j] = drand48();
        }
    }

    //Batch de imagenes salida
    Img batch_out[numImg];
    int mB, nB, sizeB;

    //Calcular tamaño matriz B
    mB = mA - mF + 1;
    nB = nA - nF + 1;
    sizeB = mB * nB;

    //Reservar memoria
    for (int ind=0; ind < numImg; ind++){    
        batch_out[ind].R = (double *) malloc(sizeof(double) * sizeB); 
        batch_out[ind].G = (double *) malloc(sizeof(double) * sizeB); 
        batch_out[ind].B = (double *) malloc(sizeof(double) * sizeB); 
    }    
            
    /* ****** Cálculo de convolución ****** */ 

    double elapsed, ucpu, scpu;
    ctimer(&elapsed, &ucpu, &scpu);
    for (int ind=0; ind < numImg; ind++){
        convolucion(batch[ind].R, mA, nA, F, mF, nF, batch[ind].R);
        convolucion(batch[ind].G, mA, nA, F, mF, nF, batch[ind].G);
        convolucion(batch[ind].B, mA, nA, F, mF, nF, batch[ind].B);
    }    
    ctimer(&elapsed, &ucpu, &scpu);

    /* ************************************ */       
    
    printf("\nTiempo =  %lf segundos\n", elapsed);
    
    for (int ind=0; ind < numImg; ind++){
        free(batch[ind].R);
        free(batch[ind].G);
        free(batch[ind].B);
        free(batch_out[ind].R);
        free(batch_out[ind].G);
        free(batch_out[ind].B);
    }
    free(F);    
    // free(Bsol);
    return 0;
}

/* Algoritmo de convolución */
void convolucion(const double *A, int mA, int nA, const double *F, int mF, int nF, double *B){
    /*
        Cada elemento de B de calcula como el producto elemento elemento
        de una submatriz de A y el Filtro, siendo la submatriz de A del
        tamaño que el filtro
    */

    // Tamaño de B, se puede generalizar
    int mB = mA - mF + 1;
    int nB = nA - nF + 1;

    double sum;
    // Dos bucles anidados para recorrer los elementos de B
    for (int i=0; i < mB; i++) {
        for (int j=0; j < nB; j++) {
            // Reseteamos el valor acumulado
            sum = 0.0;
            // Doble bucle para hacer el producto elemento a elemento
            // entre la submatriz de A y el filtro
            for (int k=0; k < mF; k++) {
                for (int s=0; s < nF; s++) {
                    sum += A[(i+k)*nA + j + s] * F[k*nF + s];
                }
            }
            B[i*nB + j] = sum;
        }
    }    
}

/* La matriz se almacena por filas */
int leerMat(int m, int n, int marco, double *mat, const char *filename){
    printf("m: %i, n: %i\n", m, n);
    FILE *file = fopen(filename, "r");

    if (file == NULL){
        return 1;
    }

    for (int i=marco; i < m+marco; i++){
        for (int j=marco; j < n+marco; j++){

            if (j == n-1+marco) {
                fscanf(file, "%lf", &mat[i*(n+marco*2) + j]);

            } else {
                fscanf(file, "%lf,", &mat[i*(n+marco*2) + j]);
            }
            
        }
    }

    fclose(file);

    return 0;
}

/* Almacenada por filas */
void imprimirMat(int m, int n, double *mat){
    for (int i=0; i < m; i++) {
        for (int j=0; j < n; j++) {
            printf("%lf ", mat[i*n + j]);
        }
        printf("\n");
    }
}