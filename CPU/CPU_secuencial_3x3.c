#include <stdio.h>
#include <stdlib.h>
#include "ctimer.h"

int leerMat(int, int, double *, const char *);
void imprimirMat(int, int, double *);
void convolucion(const double*, int, int, const double*, int, int, double*);

int main(){
    char filenameA[255] = "../Datos_validacion/A.txt";
    char filenameF[255] = "../Datos_validacion/F.txt";
    char filenameB[255] = "../Datos_validacion/B.txt";

    /* Matrices cuadradas, tamaños: */
    int tamA, tamF, tamB;
    tamA = 8;
    tamF = 3;
    tamB = 6;

    /* Reserva de memoria */
    double *A, *F, *B, *Bsol;
    A = (double *) malloc(sizeof(double) * tamA * tamA);
    F = (double *) malloc(sizeof(double) * tamF * tamF);
    B = (double *) malloc(sizeof(double) * tamB * tamB);
    Bsol = (double *) malloc(sizeof(double) * tamB * tamB);

    // Leer A
    int check = leerMat(tamA, tamA, A, filenameA);
    if (check) { printf("Fallo al leer archivo\n"); exit(-1); }

    // Leer F
    check = leerMat(tamF, tamF, F, filenameF);
    if (check) { printf("Fallo al leer archivo\n"); exit(-1); }

    // Leer B
    check = leerMat(tamB, tamB, B, filenameB);
    if (check) { printf("Fallo al leer archivo\n"); exit(-1); }


    /* ****** Cálculo de convolución ****** */ 

    double elapsed, ucpu, scpu;
    ctimer(&elapsed, &ucpu, &scpu);

    convolucion(A, tamA, tamA, F, tamF, tamF, Bsol);

    ctimer(&elapsed, &ucpu, &scpu);

    /* ************************************ */


    // Imprimir todo
    printf("\n****** Matriz A ********\n");
    imprimirMat(tamA, tamA, A);

    printf("\n\n****** Matriz F ********\n");
    imprimirMat(tamF, tamF, F);

    printf("\n\n****** Matriz B Validación ********\n");
    imprimirMat(tamB, tamB, B);

    printf("\n\n****** Matriz B Solución *********\n");
    imprimirMat(tamB, tamB, Bsol);


    // Calculo del error acumulado
    double err = 0.0;
    for (int i=0; i < tamB; i++){
        for (int j=0; j < tamB; j++){
            err += abs(B[i*tamB + j] - Bsol[i*tamB + j]);
        }
    }

    printf("\nError acumulado = %lf\n", err);
    printf("\nTiempo =  %lf segundos\n", elapsed);



    free(A);
    free(F);
    free(B);
    free(Bsol);
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
int leerMat(int m, int n, double *mat, const char *filename){

    FILE *file = fopen(filename, "r");

    if (file == NULL){
        return 1;
    }

    for (int i=0; i < m; i++){
        for (int j=0; j < n; j++){

            if (j == n-1) {
                fscanf(file, "%lf", &mat[i*n + j]);

            } else {
                fscanf(file, "%lf,", &mat[i*n + j]);
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

