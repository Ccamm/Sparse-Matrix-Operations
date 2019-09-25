#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <stdio.h>

#include "../smops.h"

#define OP MATRIX_MULT

void sequential_mult_to_dense(MATRIX_DATA *dense_matrix, MATRIX_DATA a, MATRIX_DATA b, TYPE type, int pos)
{
    switch(type) {
        case FLOAT:
            dense_matrix[pos].f += a.f*b.f;
            break;
        case INT:
            dense_matrix[pos].i += a.i*b.i;
            break;
        default:
            break;
    }
}

void parallel_mult_to_dense(MATRIX_DATA *dense_matrix, MATRIX_DATA a, MATRIX_DATA b, TYPE type, int pos)
{
    switch(type) {
        case FLOAT:
            #pragma omp atomic
            dense_matrix[pos].f += a.f*b.f;
            break;
        case INT:
            #pragma omp atomic
            dense_matrix[pos].i += a.i*b.i;
            break;
        default:
            break;
    }
}

void sequential_multiplication(MATRIX_DATA *dense_matrix, CSR_DATA *csr_a,
                            CSC_DATA *csc_b, TYPE type, int rows_result, int cols_result)
{
    int p_a, q_a, p_b, q_b, index;
    for(int r = 0; r < rows_result; r++) {
        for(int c = 0; c < cols_result; c++) {
            index = r*rows_result + c;
            p_a = csr_a->ia[r];
            q_a = csr_a->ia[r+1];
            p_b = csc_b->ia[c];
            q_b = csc_b->ia[c+1];

            for(int i = p_a; i < q_a; i++) {
                for(int j = p_b; j < q_b; j++) {
                    if(csr_a->ja[i] == csc_b->ja[j]) {
                        sequential_mult_to_dense(dense_matrix, csr_a->nnz[i], csc_b->nnz[j], type, index);
                        break;
                    }
                }
            }
        }
    }
}

void parallel_multiplication(SMOPS_CTX *ctx, MATRIX_DATA *dense_matrix, CSR_DATA *csr_a,
                            CSC_DATA *csc_b, TYPE type, int rows_result, int cols_result)
{
    MATRIX_DATA *a_nnz = csr_a->nnz;
    int *a_ia = csr_a->ia;
    int *a_ja = csr_a->ja;

    MATRIX_DATA *b_nnz = csc_b->nnz;
    int *b_ia = csc_b->ia;
    int *b_ja = csc_b->ja;

    #pragma omp parallel num_threads(ctx->thread_num) firstprivate(type, rows_result, cols_result)
    {
        #pragma omp single
        {
            int p_a, q_a, p_b, q_b, index;
            for(int r = 0; r < rows_result; r++) {
                for(int c = 0; c < cols_result; c++) {
                    #pragma omp task firstprivate(r,c)
                    {
                        index = r*rows_result + c;
                        p_a = a_ia[r];
                        q_a = a_ia[r+1];
                        p_b = b_ia[c];
                        q_b = b_ia[c+1];

                        for(int i = p_a; i < q_a; i++) {
                            for(int j = p_b; j < q_b; j++) {
                                if(a_ja[i] == b_ja[j]) {
                                    parallel_mult_to_dense(dense_matrix, a_nnz[i], b_nnz[j], type, index);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int multiplication(SMOPS_CTX *ctx, MATRIX *matrix_a, MATRIX *matrix_b)
{
    TYPE type = matrix_a->type;

    int rows_result = matrix_a->rows;
    int cols_result = matrix_b->cols;
    int size_result = rows_result*cols_result;

    CSR_DATA *csr_a = matrix_a->csr_data;
    CSC_DATA *csc_b = matrix_b->csc_data;

    MATRIX_DATA *dense_matrix = (MATRIX_DATA *)calloc(size_result, sizeof(MATRIX_DATA));
    if(dense_matrix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for dense matrix operation");
        return 0;
    }

    switch(ctx->thread_num) {
        case 1:
            sequential_multiplication(dense_matrix, csr_a, csc_b, type, rows_result, cols_result);
            break;
        default:
            parallel_multiplication(ctx, dense_matrix, csr_a, csc_b, type, rows_result, cols_result);
            break;
    }
    SMOPS_RESULT_save_matrix_result(ctx, dense_matrix, type, rows_result, cols_result);
    return 1;
}

int MATRIX_OP_multiplication(SMOPS_CTX *ctx, MATRIX *matrix_a, MATRIX *matrix_b)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    if(OPS_check_format(ctx, matrix_a, OP, NONE) == 0) {return 0;}
    if(OPS_check_format(ctx, matrix_b, OP, CSC) == 0) {return 0;}

    if(matrix_a->rows != matrix_b->cols || matrix_a->cols != matrix_b->rows) {
        SMOPS_CTX_fill_err_msg(ctx, "input matrices for multiplication do not have the correct dimensions");
        return 0;
    }

    if(matrix_a->type != matrix_b->type || matrix_a->type == UNDEFINED) {
        SMOPS_CTX_fill_err_msg(ctx, "type not properly set for matrices used in multiplication");
        return 0;
    }

    if(multiplication(ctx, matrix_a, matrix_b) == 0) {return 0;}

    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_op = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    printf("clock time: %f\n", ctx->time_op);
    return 1;
}
