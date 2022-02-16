#include <stdio.h>
#include <stdlib.h>
#include "ctimer.h"
#include <assert.h>

void imprimirMat(int, int, double *);
void convolucion(const double*, int, int, const double*, int, int, double*);
int leerMat(int m, int n, int marco, double *mat, const char *filename);

int main( int argc, char *argv[] ){

    if ( argc < 5 ){
        printf("\nUsage: %s mA nA mF nF [padding=0]\n", argv[0]);
        exit(-1);
    }
    int padding = 0;

    /* Padding */
    if ( (argc == 6) && (atoi(argv[5]) == 1) ) {
        padding = 1;
    }

    unsigned int mA, nA, mF, nF;
    mA = atoi(argv[1]);
    nA = atoi(argv[2]);
    mF = atoi(argv[3]);
    nF = atoi(argv[4]);

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

    /* Reserva de memoria */
    double *A, *F;
    A = (double *) calloc(sizeA, sizeof(double)); //A se llena de ceros
    F = (double *) malloc(sizeof(double) * sizeF);
    drand48(); //El primer valor da 0 xdd ??
    /* Generación de las matrices A y F */
    for (int i=marco; i < mA-marco; i++){
        for (int j=marco; j < nA-marco; j++){
            A[i*nA + j] = drand48();
        }
    }

    for (int i=0; i < mF; i++){
        for (int j=0; j < nF; j++){
            F[i*nF + j] = drand48();
        }
    }

    //Cargar matrices
    // int check = leerMat(mA-marco*2, nA-marco*2, marco, A, "../Datos_validacion/A.txt");
    // if (check) { printf("Fallo al leer archivo\n"); exit(-1); }
    // check = leerMat(mF, nF, 0, F, "../Datos_validacion/F.txt");
    // if (check) { printf("Fallo al leer archivo\n"); exit(-1); }

    printf("\n******* Matriz A ********\n");
    imprimirMat(mA, nA, A);

    printf("\n******* Matriz F ********\n");
    imprimirMat(mF, nF, F);


    /* Reserva de memoria de B, en función de si el padding está activado */
    double *B;
    int mB, nB, sizeB;

    // if ( padding ) {
    //     mB = mA;
    //     nB = nA;
    //     sizeB = sizeA;
    // } else {
    //     mB = mA - mF + 1;
    //     nB = nA - nF + 1;
    //     sizeB = mB * nB;
    // }
    //Calcular tamaño matriz B
    mB = mA - mF + 1;
    nB = nA - nF + 1;
    sizeB = mB * nB;

    //Reservar memoria y cargar matriz B
    B = (double *) malloc(sizeof(double) * sizeB);
    // double *Bsol = (double *) malloc(sizeof(double) * sizeB);
    // check = leerMat(mB, nB, 0, Bsol, "../Datos_validacion/Bp.txt");
    // if (check) { printf("Fallo al leer archivo\n"); exit(-1); }

    /* ****** Cálculo de convolución ****** */ 

    double elapsed, ucpu, scpu;
    ctimer(&elapsed, &ucpu, &scpu);

    convolucion(A, mA, nA, F, mF, nF, B);

    ctimer(&elapsed, &ucpu, &scpu);

    /* ************************************ */

    printf("\n******* Matriz B ********\n");
    imprimirMat(mB, nB, B);
    // printf("\n******* Solucion ********\n");
    // imprimirMat(mB, nB, Bsol);


    // Calculo del error acumulado
    // double err = 0.0;
    // for (int i=0; i < mB; i++){
    //     for (int j=0; j < nB; j++){
    //         err += abs(B[i*nB + j] - Bsol[i*nB + j]);
    //     }
    // }

    // printf("\nError acumulado = %lf\n", err);
    printf("\nTiempo =  %lf segundos\n", elapsed);



    free(A);
    free(F);
    free(B);
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