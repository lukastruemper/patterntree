#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 8192
#define K 256
#define M 4096

__global__
void mxv(double* a, double* b, double* c) {
    int blockId = blockIdx.x + blockIdx.y * gridDim.x;
    int row = threadIdx.x + blockId * (blockDim.x * blockDim.y);    

    for (int i = 0; i < M; i++)
    {
        c[row * M + i] = 0.0;
        for (int j = 0; j < N; j++) {
            c[row * M + i] += a[row * N + j] * b[i * N + j];
        }
    }
}

int main(int argc, char** argv)
{
    // 1. Memory allocation
    double* a_h = (double *) malloc(N * N * sizeof(double));
    double* b_h = (double *) malloc(M * N * sizeof(double));
    double* c_h = (double *) malloc(M * N * sizeof(double));
    
    double* a_d;
    double* b_d;
    double* c_d;
    cudaMalloc(&a_d, (N - K) * N * sizeof(double));
    cudaMalloc(&b_d, M * N * sizeof(double));
    cudaMalloc(&c_d, M * (N - K) * sizeof(double));


    printf("Allocation done\n");

    // 2. Random initialization
    int seed = 42;
    srand(seed);
    for (int i = 0; i < N * N; i++) {
        a_h[i] = 2.0;
        //m_h[i] = ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0;
    }
    for (int i = 0; i < N * M; i++) {
        b_h[i] = 4.0;
    }

    printf("Random init done\n");

    printf("Starting clock\n");
    // 3. Start clock
    double time = omp_get_wtime();
    
    cudaMemcpy(a_d, a_h, (N - K) * N * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(b_d, b_h, M * N * sizeof(double), cudaMemcpyHostToDevice);

    mxv<<<(N - K) / 256, 256>>>(a_d, b_d, c_d);

    #pragma omp parallel for
    for (int i = N - K; i < N; i++) {
        for (int k = 0; k < M; k++) {
            c_h[i * M + k] = 0.0;
            for (int j = 0; j < N; j++) {
                c_h[i * M + k] += a_h[i * N + j] * b_h[k * N + j];
            }
        }
    }

    cudaMemcpy(c_h, c_d, M * (N - K) * sizeof(double), cudaMemcpyDeviceToHost);

    // 4. Stop time
    time = omp_get_wtime() - time;
    printf("Stopping clock\n");
    
    // 5. Computation for non-trivialization of code
    for (int i = 0; i < M * N; i++) {
        if (c_h[i] != N * 8.0) {
            printf("ERRROR: %f!", c_h[i]);
        }
    }

    printf("Computation took: %f\n", time);
      
    cudaFree(a_d);
    cudaFree(b_d);
    cudaFree(c_d);
    free(a_h);
    free(b_h);
    free(c_h);
    
    return 0;
}
