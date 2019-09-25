#include <stdlib.h>
#include <omp.h>
#include <time.h>

#include "../smops.h"

#define OP SCALAR_MULT

int MATRIX_OP_scalar_multiplication(SMOPS_CTX *ctx, MATRIX *result, MATRIX *matrix, double sm)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    if(OPS_check_format(ctx, matrix, OP, NONE) == 0) {return 0;}
    if(matrix->type != FLOAT) {
        SMOPS_CTX_fill_err_msg(ctx, "input matrix does not have type float for scalar multiplication");
        return 0;
    }
    if(MATRIX_set_properties(ctx, result, matrix->format, matrix->type,
        matrix->rows, matrix->cols, matrix->non_zero_size) == 0) {return 0;}

    int non_zero_size = result->non_zero_size;
    result->coo_data->coords_i = (int *)calloc(non_zero_size, sizeof(int));
    if(result->coo_data->coords_i == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for COO_DATA");
        return 0;
    }
    result->coo_data->coords_j = (int *)calloc(non_zero_size, sizeof(int));
    if(result->coo_data->coords_j == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for COO_DATA");
        return 0;
    }
    result->coo_data->values = (MATRIX_DATA *)calloc(non_zero_size, sizeof(MATRIX_DATA));
    if(result->coo_data->values == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for COO_DATA");
        return 0;
    }

    MATRIX_DATA *r_values = result->coo_data->values;
    int *r_coords_i = result->coo_data->coords_i;
    int *r_coords_j = result->coo_data->coords_j;

    MATRIX_DATA *m_values = matrix->coo_data->values;
    int *m_coords_i = matrix->coo_data->coords_i;
    int *m_coords_j = matrix->coo_data->coords_j;

    switch(ctx->thread_num) {
        case 1:
            for(int i = 0; i < non_zero_size; i++) {
                r_coords_i[i] = m_coords_i[i];
                r_coords_j[i] = m_coords_j[i];
                r_values[i].f = m_values[i].f*sm;
            }
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num)
            {
                int i;
                #pragma omp for
                for(i = 0; i < non_zero_size; i++) {
                    r_coords_i[i] = m_coords_i[i];
                    r_coords_j[i] = m_coords_j[i];
                    r_values[i].f = m_values[i].f*sm;
                }
            }
            break;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_op = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    SMOPS_RESULT_save_coo_matrix_result(ctx, result);
    return 1;
}
