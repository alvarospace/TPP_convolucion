#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CUDA_SAFE_CALL( call ) {                                         \
 cudaError_t err = call;                                                 \
 if( cudaSuccess != err ) {                                              \
   fprintf(stderr,cudaGetErrorString(err)); \
   exit(err);                                                            \
 } }

int leerMat(int, int, double *, const char *);
void imprimirMat(int, int, double *);
void convolucion(const double*, int, int, const double*, int, int, double*);

__global__ void compute_kernel( unsigned int mB, unsigned int nB,unsigned int mF,unsigned int nF, unsigned int nA, double *d_A, double *d_F, double *d_B ) {

  /* Obtain the global matrix index accessed by the thread executing this kernel */
  int i = blockIdx.x * blockDim.x + threadIdx.x; // Global row index
  int j = blockIdx.y * blockDim.y + threadIdx.y; // Global column index

  if( i<mB && j<nB ) {
    // Reseteamos el valor acumulado
    double sum = 0.0;
    // Doble bucle para hacer el producto elemento a elemento
    // entre la submatriz de A y el filtro
    for (int k=0; k < mF; k++) {
        for (int s=0; s < nF; s++) {
            sum += d_A[(i+k)*nA + j + s] * d_F[k*nF + s];
        }
    }
    d_B[i*nB + j] = sum;
  } 
}


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
    A = (double *) malloc(sizeof(double) * sizeA);
    F = (double *) malloc(sizeof(double) * sizeF);

    /* Generación de las matrices A y F */
    //drand48(); //El primer valor da 0 xdd ??
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

    // printf("\n******* Matriz A ********\n");
    // imprimirMat(mA, nA, A);

    // printf("\n******* Matriz F ********\n");
    // imprimirMat(mF, nF, F);


    /* Reserva de memoria de B, en función de si el padding está activado */
    double *B;
    int mB, nB, sizeB;

    mB = mA - mF + 1;
    nB = nA - nF + 1;
    sizeB = mB * nB;

    B = (double *) malloc(sizeof(double) * sizeB);


    /* ****** Cálculo de convolución ****** */ 

    convolucion(A, mA, nA, F, mF, nF, B);

    /* ************************************ */

    // printf("\n******* Matriz B ********\n");
    // imprimirMat(mB, nB, B);
    FILE *temp=fopen("temp.txt","w");
    if(temp == NULL)
    {
      printf("Error!");   
      exit(1);             
    }
    for(int i=0; i<nB; i++){
        for(int j=0; j<mB; j++){
            double val = B[i*nB + j];
            fprintf(temp,"%f", val);
            fprintf(temp,"%s", " ");
        }
        fprintf(temp,"\n");
    }
    fclose(temp);


    // // Calculo del error acumulado
    // double err = 0.0;
    // for (int i=0; i < tamB; i++){
    //     for (int j=0; j < tamB; j++){
    //         err += abs(B[i*tamB + j] - Bsol[i*tamB + j]);
    //     }
    // }

    // printf("\nError acumulado = %lf\n", err);
    // printf("\nTiempo =  %lf segundos\n", elapsed);



    free(A);
    free(F);
    free(B);
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

    unsigned int mem_size_A = mA * nA * sizeof(double);
    unsigned int mem_size_F = mF * nF * sizeof(double);
    unsigned int mem_size_B = mB * nB * sizeof(double);

    printf("Allocating memory\n");

    double *d_A, *d_F, *d_B;
    CUDA_SAFE_CALL( cudaMalloc((void **) &d_A, mem_size_A ) );
    CUDA_SAFE_CALL( cudaMalloc((void **) &d_F, mem_size_F ) );
    CUDA_SAFE_CALL( cudaMalloc((void **) &d_B, mem_size_B ) );

    printf("Coping HOST to DEVICE\n");

    CUDA_SAFE_CALL( cudaMemcpy( d_A, A, mem_size_A, cudaMemcpyHostToDevice ) );
    CUDA_SAFE_CALL( cudaMemcpy( d_F, F, mem_size_F, cudaMemcpyHostToDevice ) );
    CUDA_SAFE_CALL( cudaMemcpy( d_B, B, mem_size_B, cudaMemcpyHostToDevice ) );

    unsigned int nthreadsX = 8;
    unsigned int nthreadsY = ceil(nthreadsX*mB/nB);

    unsigned int nblocksX = ceil(mB/nthreadsX) + 1;
    unsigned int nblocksY = ceil(nB/nthreadsY) + 1;

    printf("nthreadsX: %d\n", nthreadsX);
    printf("nthreadsY: %d\n", nthreadsY);

    printf("nblocksX: %d\n", nblocksX);
    printf("nblocksY: %d\n", nblocksY);

    dim3 dimGrid( nblocksX, nblocksY );
    dim3 dimBlock( nthreadsX, nthreadsY );

    printf("Executing kernel\n");

    cudaEvent_t start, stop;
    float elapsedTime;

    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);
    compute_kernel<<< dimGrid, dimBlock >>>( mB, nB, mF, nF, nA, d_A, d_F, d_B );

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    cudaEventElapsedTime(&elapsedTime, start,stop);
    printf("Time = %f ms\n",elapsedTime);

    // printf("Coping DEVICE to HOST\n");

    // CUDA_SAFE_CALL( cudaMemcpy( B, d_B, mem_size_B, cudaMemcpyDeviceToHost ) );

    // Deallocate device memory
    CUDA_SAFE_CALL( cudaFree(d_A) );
    CUDA_SAFE_CALL( cudaFree(d_F) );
    CUDA_SAFE_CALL( cudaFree(d_B) );
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

