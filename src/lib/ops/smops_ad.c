#include <stdlib.h>
#include <omp.h>
#include <time.h>

#include "../smops.h"

#define OP ADD

/** Adds the element to dense matrix arbitrary to type
*
*   paramaters:
*       MATRIX_DATA *dense_matrix: the dense matrix where the element is added
*       MATRIX_DATA elem: the element to add
*       TYPE: the type the data is
*       int pos: the 1D position of the element being added
*/
void add_to_dense_elem(MATRIX_DATA *dense_matrix, MATRIX_DATA elem, TYPE type, int pos)
{
    switch(type) {
        case FLOAT:
            #pragma omp atomic
            dense_matrix[pos].f += elem.f;
            break;
        case INT:
            #pragma omp atomic
            dense_matrix[pos].i += elem.i;
            break;
        default:
            break;
    }
}

/** Performs the addition on matrix_a and matrix_b returning the result as a dense matrix
*   dense_matrix = matrix_a + matrix_b
*
*   paramaters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error management
*       MATRIX *matrix_a: one of the matrices that is added together
*       MATRIX *matrix_b: the other matrix that is added together
*
*   return:
*       the result of matrix_a + matrix_b as the form of a dense 1D matrix
*/
int addition(SMOPS_CTX *ctx, MATRIX *matrix_a, MATRIX *matrix_b)
{
    TYPE type = matrix_a->type;

    int rows = matrix_a->rows;
    int size = matrix_a->size;

    CSR_DATA *csr_a = matrix_a->csr_data;
    CSR_DATA *csr_b = matrix_b->csr_data;

    MATRIX_DATA *dense_matrix = (MATRIX_DATA *)calloc(size, sizeof(MATRIX_DATA));
    if(dense_matrix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for dense matrix in addition");
        return 0;
    }

    #pragma omp parallel num_threads(ctx->thread_num) firstprivate(type, rows)
    {
        #pragma omp single
        {
            int p_a, q_a, p_b, q_b;
            for(int r = 1; r < rows + 1; r++) {
                #pragma omp task firstprivate(r)
                {
                    p_a = csr_a->ia[r-1];
                    q_a = csr_a->ia[r];
                    for(int i = p_a; i < q_a; i++) {
                        add_to_dense_elem(dense_matrix, csr_a->nnz[i], type, (r-1)*rows + csr_a->ja[i]);
                    }
                }

                #pragma omp task firstprivate(r)
                {
                    p_b = csr_b->ia[r-1];
                    q_b = csr_b->ia[r];
                    for(int i = p_b; i < q_b; i++) {
                        add_to_dense_elem(dense_matrix, csr_b->nnz[i], type, (r-1)*rows + csr_b->ja[i]);
                    }
                }
            }
        }
    }
    SMOPS_RESULT_save_matrix_result(ctx, dense_matrix, type, rows, matrix_a->cols);
    return 1;
}

/** Performs the addition on matrix_a and matrix_b returning the result as a dense matrix
*   dense_matrix = matrix_a + matrix_b
*
*   paramaters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for error management
*       MATRIX *matrix_a: one of the matrices that is added together
*       MATRIX *matrix_b: the other matrix that is added together
*
*   return:
*       1 if successfully executed, 0 otherwise filling error message
*/
int MATRIX_OP_addition(SMOPS_CTX *ctx, MATRIX *matrix_a, MATRIX *matrix_b)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    if(OPS_check_format(ctx, matrix_a, OP, NONE) == 0) {return 0;}
    if(OPS_check_format(ctx, matrix_b, OP, NONE) == 0) {return 0;}

    if(matrix_a->rows != matrix_b->rows || matrix_a->cols != matrix_b->cols) {
        SMOPS_CTX_fill_err_msg(ctx, "input matrices for addition do not have same dimensions");
        return 0;
    }

    if(matrix_a->type != matrix_b->type || matrix_a->type == UNDEFINED) {
        SMOPS_CTX_fill_err_msg(ctx, "type not properly set for matrices used in addition");
        return 0;
    }

    if(addition(ctx, matrix_a, matrix_b) == 0) {return 0;}
    
    clock_gettime(CLOCK_REALTIME, &end);
    ctx->time_op = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec)/ BILLION;
    return 1;
}
