#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>


#define MAX_SIZE 20

typedef struct {
    int rows;
    int cols;
    int matrix[MAX_SIZE][MAX_SIZE];
} Matrix;

typedef struct {
    Matrix *a;
    Matrix *b;
    Matrix *c;
    int row ;
    int column ;
} ThreadArgsForElementMultiplication;

typedef struct{
    Matrix *a;
    Matrix *b;
    Matrix *c;
    int row ;
} ThreadArgsForRowMultiplication;

void *threadFunctionPerRow(void *args);
void *threadFunctionPerElement(void *args);
void multiplyMatrixPerMatrix(Matrix *a, Matrix *b, Matrix *c);
void multiplyMatrixPerRow(Matrix *a, Matrix *b, Matrix *c);
void multiplyMatrixPerElement(Matrix *a, Matrix *b, Matrix *c);
void readMatrixFromFile(const char *fileName, Matrix *matrix);
void writeMatrixToFile(const char *fileName, Matrix *matrix);

int numThreadsForMatrixMultiplication = 0;
int numThreadsForRowMultiplication=0 ;
int numThreadsForElementMultiplication = 0;
int main(int argc, char *argv[]) {

    const char *inputFileA = (argc > 1) ? argv[1] : "a.txt";
    const char *inputFileB = (argc > 2) ? argv[2] : "b.txt";
    const char *outputPrefix = (argc > 3) ? argv[3] : "c";

    Matrix matrixA, matrixB, matrixC;
    readMatrixFromFile(inputFileA, &matrixA);
    readMatrixFromFile(inputFileB, &matrixB);


    matrixC.rows = matrixA.rows;
    matrixC.cols = matrixB.cols;
    matrixC.matrix[matrixC.rows][matrixC.cols];
    for (int i = 0; i < matrixC.rows; i++) {
        for (int j = 0; j < matrixC.cols; j++) {
            matrixC.matrix[i][j] = 0;
        }
    }

    struct timeval stop, start;

    printf("Matrix Multiplication\n");

    gettimeofday(&start, NULL); //start checking time
    multiplyMatrixPerMatrix(&matrixA, &matrixB, &matrixC);
    gettimeofday(&stop, NULL); //end checking time

    printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
    printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    printf("Method: A thread per matrix  : %d\n", numThreadsForMatrixMultiplication);
    writeMatrixToFile("c_per_matrix.txt", &matrixC);


    for (int i = 0; i < matrixC.rows; i++) {
        for (int j = 0; j < matrixC.cols; j++) {
            matrixC.matrix[i][j] = 0;

        }
    }
    printf("\nRow Multiplication\n");

    struct timeval stop2, start2;
    gettimeofday(&start2, NULL); //start checking time
    multiplyMatrixPerRow(&matrixA, &matrixB, &matrixC);
    gettimeofday(&stop2, NULL); //end checking time
    printf("Seconds taken %lu\n", stop2.tv_sec - start2.tv_sec);
    printf("Microseconds taken: %lu\n", stop2.tv_usec - start2.tv_usec);
    writeMatrixToFile("c_per_row.txt", &matrixC);
    printf("Method: A thread per row : %d\n", numThreadsForRowMultiplication);
    for (int i = 0; i < matrixC.rows; i++) {
        for (int j = 0; j < matrixC.cols; j++) {
            matrixC.matrix[i][j] = 0;
        }
    }


    printf("\nElement Multiplication\n");
    struct timeval stop3, start3;

    gettimeofday(&start3, NULL); //start checking time
    multiplyMatrixPerElement(&matrixA, &matrixB, &matrixC);
    gettimeofday(&stop3, NULL); //end checking time
    printf("Seconds taken %lu\n", stop3.tv_sec - start3.tv_sec);
    printf("Microseconds taken: %lu\n", stop3.tv_usec - start3.tv_usec);
    writeMatrixToFile("c_per_element.txt", &matrixC);
    printf("Method: A thread per element : %d\n", numThreadsForElementMultiplication);

    return 0;
}

void readMatrixFromFile(const char *fileName, Matrix *matrix) {
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "row=%d col=%d", &(matrix->rows), &(matrix->cols)) != 2) {
        fprintf(stderr, "Invalid matrix format in %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    int i, j;
    for (i = 0; i < matrix->rows; i++) {
        for (j = 0; j < matrix->cols; j++) {
            if (fscanf(file, "%d", &(matrix->matrix[i][j])) != 1) {
                fprintf(stderr, "Error reading matrix data from %s\n", fileName);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
}
void writeMatrixToFile(const char *fileName, Matrix *matrix) {
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Method: ");
    if (fileName[0] == 'c' && fileName[6] == 'm') {
        fprintf(file, "A thread per matrix\n");
    } else if (fileName[0] == 'c' && fileName[6] == 'r') {
        fprintf(file, "A thread per row\n");
    } else if (fileName[0] == 'c' && fileName[6] == 'e') {
        fprintf(file, "A thread per element\n");
    }

    fprintf(file, "row=%d col=%d\n", matrix->rows, matrix->cols);
    int i, j;
    for (i = 0; i < matrix->rows; i++) {
        for (j = 0; j < matrix->cols; j++) {
            fprintf(file, "%d ", matrix->matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}



void multiplyMatrixPerMatrix(Matrix *a, Matrix *b, Matrix *c){
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            c->matrix[i][j] = 0;
            for (int k = 0; k < b->rows; k++) {
                c->matrix[i][j] += a->matrix[i][k] * b->matrix[k][j];
            }
        }
    }
}


void multiplyMatrixPerElement(Matrix *a, Matrix *b, Matrix *c) {
    pthread_t t[a->rows][b->cols];
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            ThreadArgsForElementMultiplication *args = malloc(sizeof(ThreadArgsForElementMultiplication));
            args->a = a ; args->b = b ; args->c = c ; args->row = i ;
            args->column = j ;
            pthread_create(&t[i][j], NULL, &threadFunctionPerElement, args);
        }}
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            pthread_join(t[i][j], NULL);
        }
    }
    numThreadsForElementMultiplication = a->rows * b->cols;
}
void *threadFunctionPerElement(void *args){
    ThreadArgsForElementMultiplication *arg = (ThreadArgsForElementMultiplication *) args;
    arg->c->matrix[arg->row][arg->column] = 0;
    for (int k = 0; k < arg->b->rows; k++) {
        arg->c->matrix[arg->row][arg->column] += arg->a->matrix[arg->row][k] * arg->b->matrix[k][arg->column];
    }
    free(arg);
    pthread_exit(NULL);
}
void multiplyMatrixPerRow(Matrix *a, Matrix *b, Matrix *c){
    pthread_t t[a->rows];
    for (int i = 0; i < a->rows; i++) {
        ThreadArgsForRowMultiplication *args = malloc(sizeof(ThreadArgsForRowMultiplication));
        args->a = a; args->b =b ; args->c = c ;
        args->row = i;
        pthread_create(&t[i], NULL, threadFunctionPerRow, args);
    }
    for (int i = 0; i < a->rows; i++) {
        pthread_join(t[i], NULL);
    }
    numThreadsForRowMultiplication = a->rows;
}

void *threadFunctionPerRow(void *args){
    ThreadArgsForRowMultiplication *arg = (ThreadArgsForRowMultiplication *) args ;
    for (int j = 0; j < arg->b->cols; j++) {
        arg->c->matrix[arg->row][j] = 0 ;
        for (int k = 0; k < arg->b->rows; k++) {
            arg->c->matrix[arg->row][j] += arg->a->matrix[arg->row][k] * arg->b->matrix[k][j];
        }
    }
    free(arg);
    pthread_exit(NULL);
}