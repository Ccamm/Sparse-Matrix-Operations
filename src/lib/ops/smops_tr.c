#include <stdlib.h>
#include <omp.h>
#include <time.h>

#include "../smops.h"

#define OP TRACE

/** Finds the trace of the matrix of data type float
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX_DATA *result: where the result is stored
*       MATRIX *matrix: the matrix to calculate the trace from
*
*   return:
*       1 if executed successfully, 0 otherwise filling the error message
*/
int float_trace(SMOPS_CTX *ctx, MATRIX_DATA *result, MATRIX *matrix)
{
    double trace = 0;
    int non_zero_size = matrix->non_zero_size;
    if(matrix->coo_data ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA not set for matrix and cannot find trace");
        return 0;
    }
    int *coords_i = matrix->coo_data->coords_i;
    if(coords_i ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    int *coords_j = matrix->coo_data->coords_j;
    if(coords_j ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    MATRIX_DATA *values = matrix->coo_data->values;
    if(values ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    switch(ctx->thread_num) {
        case 1:
            for(int i = 0; i < non_zero_size; i++) {
                if(coords_i[i] == coords_j[i]) {
                    trace += values[i].f;
                }
            }
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num) reduction(+ : trace)
            {
                int i;
                #pragma omp for
                for(i = 0; i < non_zero_size; i++) {
                    if(coords_i[i] == coords_j[i]) {
                        trace += values[i].f;
                    }
                }
            }
            break;
    }
    result[0].f = trace;
    return 1;
}

/** Finds the trace of the matrix of data type int
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX_DATA *result: where the result is stored
*       MATRIX *matrix: the matrix to calculate the trace from
*
*   return:
*       1 if executed successfully, 0 otherwise filling the error message
*/
int int_trace(SMOPS_CTX *ctx, MATRIX_DATA *result, MATRIX *matrix)
{
    int trace = 0;
    int non_zero_size = matrix->non_zero_size;
    if(matrix->coo_data ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA not set for matrix and cannot find trace");
        return 0;
    }
    int *coords_i = matrix->coo_data->coords_i;
    if(coords_i ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    int *coords_j = matrix->coo_data->coords_j;
    if(coords_j ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    MATRIX_DATA *values = matrix->coo_data->values;
    if(values ==  NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "COO DATA improperly set for trace");
        return 0;
    }
    switch(ctx->thread_num) {
        case 1:
            for(int i = 0; i < non_zero_size; i++) {
                if(coords_i[i] == coords_j[i]) {
                    trace += values[i].i;
                }
            }
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num) reduction(+ : trace)
            {
                int i;
                #pragma omp for
                for(i = 0; i < non_zero_size; i++) {
                    if(coords_i[i] == coords_j[i]) {
                        trace += values[i].i;
                    }
                }
            }
            break;
    }
    result[0].i = trace;
    return 1;
}

/** Finds the trace of the matrix
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error handling
*       MATRIX_DATA *result: where the result is stored
*       MATRIX *matrix: the matrix to calculate the trace from
*
*   return:
*       1 if executed successfully, 0 otherwise filling the error message
*/
int MATRIX_OP_trace(SMOPS_CTX *ctx, MATRIX_DATA *result, MATRIX *matrix)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    if(OPS_check_format(ctx, matrix, OP, NONE) == 0) {return 0;}
    if(matrix->rows != matrix->cols) {
        SMOPS_CTX_fill_err_msg(ctx, "cannot find a trace of a non-square matrix");
        return 0;
    }
    switch(matrix->type) {
        case INT:
            if(int_trace(ctx, result, matrix) == 0) {return 0;}
            break;
        case FLOAT:
            if(float_trace(ctx, result, matrix) == 0) {return 0;}
            break;
        default:
            SMOPS_CTX_fill_err_msg(ctx, "matrix data type is not properly set");
            return 0;
    }
    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_op = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    SMOPS_RESULT_save_trace_result(ctx, result[0], matrix->type);
    return 1;
}
