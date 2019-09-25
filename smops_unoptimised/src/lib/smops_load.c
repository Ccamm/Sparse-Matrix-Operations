#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#include "smops.h"

#define BUFFER_SIZE 50
#define FLOAT_STR "float\0"
#define INT_STR "int\0"
#define READSPECLINE if(fgets(buffer, BUFFER_SIZE, file) == NULL)
#define DATA_CHUNK_SIZE 1000

/** Gets the data type from the string and puts it into the MATRIX data structure
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       MATRIX *matrix: the matrix to set its data type
*       char *str: the string form of the data type
*
*   return:
*       the TYPE that is stored in the matrix
*       returns UNDEFINED if it cannot determine the data type in str
*/
TYPE get_type(SMOPS_CTX *ctx, MATRIX *matrix, char *str) {
    if(strncmp(str, FLOAT_STR, strlen(FLOAT_STR)) == 0
        || SMOPS_CTX_get_operation(ctx) == SCALAR_MULT) {
            matrix->type = FLOAT;
            return FLOAT;
        }
    if(strncmp(str, INT_STR, strlen(INT_STR)) == 0) {
        matrix->type = INT;
        return INT;
    }
    matrix->type = UNDEFINED;
    return UNDEFINED;
}

/** Converts from COO format to CSR or CSC format
*
*   parameters:
*       MATRIX_DATA *nnz: the nnz array for CSR and CSC format
*       int *ia: the ia array for the CSR and CSC format
*       int *ja: the ja array for the CSR and CSC format
*       MATRIX_DATA *values: the values from COO format that will go into the nnz array
*       int *array_a: either coords_i or coords_j from COO format,
*                     depending on converting to CSR or CSC format
*       int non_zero_size: number of non zero elements in the matrix
*       int n: either number of rows or cols depending converting to CSR or CSC format
*       int thread_num: number of threads should be used
*/
void convert_coo(MATRIX_DATA *nnz, int *ia, int *ja, MATRIX_DATA *values,
    int *array_a, int *array_b, int non_zero_size, int n, int thread_num)
{
    int i;
    switch(thread_num) {
        case 1:
            for(i = 0; i < non_zero_size; i++) {
                nnz[i] = values[i];
                ja[i] = array_b[i];
                ia[array_a[i] + 1] += 1;
            }
            break;
        default:
            #pragma omp parallel num_threads(thread_num) private(i)
            {
                #pragma omp for
                for(i = 0; i < non_zero_size; i++) {
                    nnz[i] = values[i];
                    ja[i] = array_b[i];
                    #pragma omp atomic
                    ia[array_a[i] + 1] += 1;
                }
            }
            break;
    }

    for(int i = 1; i < n + 1; i++)
    {
        ia[i] += ia[i-1];
    }
}

/** Converts COO format to CSC format for a matrix
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to store the CSC format
*
*   return:
*       1 if successfully executed, 0 otherwise and fills in error message
*/
int coo_to_csc(SMOPS_CTX *ctx, MATRIX *matrix)
{
    int non_zero_size = matrix->non_zero_size;
    int cols = matrix->cols;
    COO_DATA *coo_data = matrix->coo_data;
    CSC_DATA *csc_data = matrix->csc_data;

    if(csc_data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "csc memory has not been set for matrix (CSC format not set)");
        return 0;
    }
    csc_data->nnz = (MATRIX_DATA *)malloc(sizeof(MATRIX_DATA)*non_zero_size);
    if(csc_data->nnz == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csc data for matrix");
        return 0;
    }
    csc_data->ia = (int *)calloc(cols + 1, sizeof(int));
    if(csc_data->ia == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csc data for matrix");
        return 0;
    }
    csc_data->ja = (int *)malloc(sizeof(int)*non_zero_size);
    if(csc_data->ja == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csc data for matrix");
        return 0;
    }

    COO_sort_col_order(coo_data, matrix->non_zero_size);

    convert_coo(csc_data->nnz, csc_data->ia, csc_data->ja, coo_data->values,
        coo_data->coords_j, coo_data->coords_i, non_zero_size, cols, ctx->thread_num);

    return 1;
}

/** Converts COO format to CSR format for a matrix
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to store the CSR format
*
*   return:
*       1 if successfully executed, 0 otherwise and fills in error message
*/
int coo_to_csr(SMOPS_CTX *ctx, MATRIX *matrix)
{
    int non_zero_size = matrix->non_zero_size;
    int rows = matrix->rows;
    COO_DATA *coo_data = matrix->coo_data;
    CSR_DATA *csr_data = matrix->csr_data;

    if(csr_data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "csr memory has not been set for matrix (CSR format not set)");
        return 0;
    }
    csr_data->nnz = (MATRIX_DATA *)calloc(non_zero_size, sizeof(MATRIX_DATA));
    if(csr_data->nnz == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csr data for matrix");
        return 0;
    }
    csr_data->ia = (int *)calloc(rows + 1, sizeof(int));
    if(csr_data->ia == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csr data for matrix");
        return 0;
    }
    csr_data->ja = (int *)malloc(sizeof(int)*non_zero_size);
    if(csr_data->ja == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csr data for matrix");
        return 0;
    }

    COO_sort_row_order(coo_data, matrix->non_zero_size);

    convert_coo(csr_data->nnz, csr_data->ia, csr_data->ja, coo_data->values,
        coo_data->coords_i, coo_data->coords_j, non_zero_size, rows, ctx->thread_num);

    return 1;
}

/** Reads the data_str and converts it to COO format with data type float
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to fill the COO format into
*       char *data_str: the data string to read and convert into COO format
*
*   return:
*       1 if execyted successfully, 0 otherwise filling error message
*/
int float_parse_data_str_to_coo(SMOPS_CTX *ctx, MATRIX *matrix, char *data_str)
{
    int size = matrix->size;
    int cols = matrix->cols;
    int non_zero_size = 0;
    MATRIX_DATA *dense_matrix = (MATRIX_DATA *)calloc(size, sizeof(MATRIX_DATA));
    if(dense_matrix ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory to temporarily store dense matrix");
        return 0;
    }
    COO_DATA *coo_data = matrix->coo_data;

    int index;
    double elem;
    char *ptr;
    char *elem_str;
    switch(ctx->thread_num) {
        case 1:
            index = 0;
            elem_str = strtok_r(data_str, " ", &ptr);
            do {
                elem = atof(elem_str);

                if(elem != 0) {
                    dense_matrix[index].f = elem;
                    non_zero_size++;
                }
                index++;
            } while((elem_str = strtok_r(NULL, " ", &ptr)) != NULL);
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num)
            {
                #pragma omp single
                {
                    index = 0;
                    elem_str = strtok_r(data_str, " ", &ptr);
                    do {
                        elem = atof(elem_str);

                        #pragma omp task firstprivate(index, elem)
                        {
                            if(elem != 0) {
                                dense_matrix[index].f = elem;
                                #pragma omp atomic
                                non_zero_size++;
                            }
                        }
                        index++;
                    } while((elem_str = strtok_r(NULL, " ", &ptr)) != NULL);
                }
            }
            break;
    }

    matrix->non_zero_size = non_zero_size;

    coo_data->coords_i = (int *)malloc(sizeof(int)*non_zero_size);
    if(coo_data->coords_i == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }
    coo_data->coords_j = (int *)malloc(sizeof(int)*non_zero_size);
    if(coo_data->coords_j == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }
    coo_data->values = (MATRIX_DATA *)malloc(sizeof(MATRIX_DATA)*non_zero_size);
    if(coo_data->values == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }

    index = 0;
    for (int i = 0; i < size; i++) {
        elem = dense_matrix[i].f;
        if(elem != 0) {
            coo_data->coords_i[index] = i / cols;
            coo_data->coords_j[index] = i % cols;
            coo_data->values[index].f = elem;
            index++;
        }
    }

    free(dense_matrix);
    return 1;
}

/** Reads the data_str and converts it to COO format with data type int
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to fill the COO format into
*       char *data_str: the data string to read and convert into COO format
*
*   return:
*       1 if execyted successfully, 0 otherwise filling error message
*/
int int_parse_data_str_to_coo(SMOPS_CTX *ctx, MATRIX *matrix, char *data_str)
{
    int size = matrix->size;
    int cols = matrix->cols;
    int non_zero_size = 0;
    MATRIX_DATA *dense_matrix = (MATRIX_DATA *)calloc(size, sizeof(MATRIX_DATA));
    if(dense_matrix ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory to temporarily store dense matrix");
        return 0;
    }
    COO_DATA *coo_data = matrix->coo_data;

    int index, elem;
    char *ptr;
    char *elem_str;
    switch(ctx->thread_num) {
        case 1:
            index = 0;
            elem_str = strtok_r(data_str, " ", &ptr);
            do {
                elem = atoi(elem_str);

                if(elem != 0) {
                    dense_matrix[index].i = elem;
                    non_zero_size++;
                }
                index++;
            } while((elem_str = strtok_r(NULL, " ", &ptr)) != NULL);
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num)
            {
                #pragma omp single
                {
                    index = 0;
                    elem_str = strtok_r(data_str, " ", &ptr);
                    do {
                        elem = atoi(elem_str);

                        #pragma omp task firstprivate(index, elem)
                        {
                            if(elem != 0) {
                                dense_matrix[index].i = elem;
                                #pragma omp atomic
                                non_zero_size++;
                            }
                        }
                        index++;
                    } while((elem_str = strtok_r(NULL, " ", &ptr)) != NULL);
                }
            }
            break;
    }

    matrix->non_zero_size = non_zero_size;

    coo_data->coords_i = (int *)malloc(sizeof(int)*non_zero_size);
    if(coo_data->coords_i == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }
    coo_data->coords_j = (int *)malloc(sizeof(int)*non_zero_size);
    if(coo_data->coords_j == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }
    coo_data->values = (MATRIX_DATA *)malloc(sizeof(MATRIX_DATA)*non_zero_size);
    if(coo_data->values == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo_data");
        return 0;
    }

    index = 0;
    for (int i = 0; i < size; i++) {
        elem = dense_matrix[i].i;
        if(elem != 0) {
            coo_data->coords_i[index] = i / cols;
            coo_data->coords_j[index] = i % cols;
            coo_data->values[index].i = elem;
            index++;
        }
    }

    free(dense_matrix);
    return 1;
}

/** Reads the data_str and converts it to COO format
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to fill the COO format into
*       char *data_str: the data string to read and convert into COO format
*
*   return:
*       1 if execyted successfully, 0 otherwise filling error message
*/
int parse_data_str_to_coo(SMOPS_CTX *ctx, MATRIX *matrix, char *data_str)
{
    switch(matrix->type) {
        case INT:
            return int_parse_data_str_to_coo(ctx, matrix, data_str);
        case FLOAT:
            return float_parse_data_str_to_coo(ctx, matrix, data_str);
        default:
            SMOPS_CTX_fill_err_msg(ctx, "no data type set for matrix");
            return 0;
    }
    SMOPS_CTX_fill_err_msg(ctx, "wot... how did you get here the switch should work :(");
    return 0;
}

int convert_from_coo(SMOPS_CTX *ctx, MATRIX *matrix)
{
    switch(matrix->format) {
        case CSR:
            return coo_to_csr(ctx, matrix);
        case CSC:
            return coo_to_csc(ctx, matrix);
        case COO:
            return 1;
        case NONE:
            SMOPS_CTX_fill_err_msg(ctx, "format is undefined for matrix");
            return 0;
    }
    SMOPS_CTX_fill_err_msg(ctx, "well something goofed badly... the switch should work :(");
    return 0;
}

/** Gets the type of the matrix specifed in the file for preloading
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix: the matrix to preload its type
*       FILE *file: file pointer to the input matrix file
*
*   return:
*       1 if successfully executed, 0 otherwise filling error message
*/
int preload_type(SMOPS_CTX *ctx, MATRIX *matrix, FILE *file)
{
    char buffer[BUFFER_SIZE];
    READSPECLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file for preloading");
        return 0;
    }

    if(get_type(ctx, matrix, buffer) == UNDEFINED) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to get data type from file for preloading");
        return 0;
    }
    return 1;
}

/** Preloads the type for matrices a and b for addition and matrix multiplication
*   If either matrix a or b is of type float then both needed to be imported as type float.
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX *matrix_a: matrix a
*       char *filename_a: the file name for matrix a
*       MATRIX *matrix_b: matrix b
*       char *filename_b: the file name for matrix b
*
*   return:
*       1 if successfully executed, 0 otherwise and fills the error message
*/
int MATRIX_preload_type(SMOPS_CTX *ctx, MATRIX *matrix_a, char *filename_a, MATRIX *matrix_b, char *filename_b)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    FILE *file_a = fopen(filename_a, "r");
    if(file_a == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to open first file for preloading");
        return 0;
    }

    FILE *file_b = fopen(filename_b, "r");
    if(file_b == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to open second file for preloading");
        fclose(file_a);
        return 0;
    }

    if(preload_type(ctx, matrix_a, file_a) == 0) {
        fclose(file_a);
        fclose(file_b);
        return 0;
    }

    if(preload_type(ctx, matrix_b, file_b) == 0) {
        fclose(file_a);
        fclose(file_b);
        return 0;
    }

    if(matrix_a->type == FLOAT || matrix_b->type == FLOAT) {
        matrix_a->type = FLOAT;
        matrix_b->type = FLOAT;
    }

    fclose(file_a);
    fclose(file_b);
    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_load += (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    return 1;
}

/** Loads the data for the matrix from the input file specified
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       MATRIX *matrix: the matrix load the data from file into
*       char *filename: the file to load the data from
*
*   return:
*       1 if data has successfully loaded, 0 otherwise
*/
int MATRIX_load(SMOPS_CTX *ctx, MATRIX *matrix, char *filename)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to open file");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    READSPECLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        fclose(file);
        return 0;
    }

    //Get type from file if not preloaded
    if(matrix->type == UNDEFINED) {
        if(get_type(ctx, matrix, buffer) == UNDEFINED) {
            SMOPS_CTX_fill_err_msg(ctx, "failed to get data type from input file");
            fclose(file);
            return 0;
        }
    }

    READSPECLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        fclose(file);
        return 0;
    }
    matrix->rows = atoi(buffer);
    READSPECLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        fclose(file);
        return 0;
    }
    matrix->cols = atoi(buffer);
    matrix->size = matrix->rows * matrix->cols;

    size_t data_size = sizeof(char)*DATA_CHUNK_SIZE;
    char *data_str = (char *)malloc(data_size);
    if((data_size = getline(&data_str, &data_size, file)) == -1) {
        SMOPS_CTX_fill_err_msg(ctx, "error occurred reading data line from file");
        fclose(file);
        return 0;
    }
    if(parse_data_str_to_coo(ctx, matrix, data_str) == 0) {
        free(data_str);
        fclose(file);
        return 0;
    }
    if(convert_from_coo(ctx, matrix) == 0) {
        free(data_str);
        fclose(file);
    }
    free(data_str);
    fclose(file);
    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_load += (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    return 1;
}
