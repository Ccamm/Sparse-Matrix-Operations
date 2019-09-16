#include <stdlib.h>
#include <string.h>

#include "smops.h"

/** Frees the COO_DATA associated with the matrix
*
*   parameters:
*       COO_DATA *coo_data: a pointer to the COO_DATA to be freed
*/
void COO_free(COO_DATA *coo_data)
{
    if(coo_data->coords_i != NULL) free(coo_data->coords_i);
    if(coo_data->coords_j != NULL) free(coo_data->coords_j);
    if(coo_data->values != NULL) free(coo_data->values);
    free(coo_data);
}

/** Frees the CSR_DATA associated with the matrix
*
*   parameters:
*       CSR_DATA *csr_data: a pointer to the CSR_DATA to be freed
*/
void CSR_free(CSR_DATA *csr_data)
{
    if(csr_data->nnz != NULL) free(csr_data->nnz);
    if(csr_data->ia != NULL) free(csr_data->ia);
    if(csr_data->ja != NULL) free(csr_data->ja);
    free(csr_data);
}

/** Frees the CSC_DATA associated with the matrix
*
*   parameters:
*       CSC_DATA *csc_data: a pointer to the CSC_DATA to be freed
*/
void CSC_free(CSC_DATA *csc_data)
{
    if(csc_data->nnz != NULL) free(csc_data->nnz);
    if(csc_data->ia != NULL) free(csc_data->ia);
    if(csc_data->ja != NULL) free(csc_data->ja);
    free(csc_data);
}

/** Frees the data associated to the matrix
*
*   parameters:
*       MATRIX *matrix: a pointer to the matrix that has its data freed
*/
void MATRIX_free_data(MATRIX *matrix)
{
    if(matrix->coo_data != NULL) COO_free(matrix->coo_data);
    if(matrix->csr_data != NULL) CSR_free(matrix->csr_data);
    if(matrix->csc_data != NULL) CSC_free(matrix->csc_data);
}

/** Frees the memory for a matrix
*
*   parameters:
*       MATRIX *matrix: the matrix to be free
*/
void MATRIX_free(MATRIX *matrix)
{
    MATRIX_free_data(matrix);
    free(matrix);
}

/** Sets the format for the Matrix
*
*   paramaters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       MATRIX *matrix: a pointer to the matrix to have its format set
*       MATRIX_FORMAT format: the format to set for the matrix
*
*   return:
*       1 if executed successfully, 0 if not and fills the err_msg in SMOPS_CTX
*/
int MATRIX_set_format(SMOPS_CTX *ctx, MATRIX *matrix, MATRIX_FORMAT format)
{
    matrix->format = format;

    matrix->coo_data = NULL;
    matrix->csr_data = NULL;
    matrix->csc_data = NULL;

    switch(matrix->format){
        case CSR:
            matrix->csr_data = (CSR_DATA *)malloc(sizeof(CSR_DATA));
            if(matrix->csr_data == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csr data for matrix");
                return 0;
            }
            break;
        case COO:
            matrix->coo_data = (COO_DATA *)malloc(sizeof(COO_DATA));
            if(matrix->coo_data == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for coo data for matrix");
                return 0;
            }
            break;
        case CSC:
            matrix->csc_data = (CSC_DATA *)malloc(sizeof(CSC_DATA));
            if(matrix->csc_data == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for csc data for matrix");
                return 0;
            }
            break;
        default:
            SMOPS_CTX_fill_err_msg(ctx, "failed to get required format for matrix");
            return 0;
    }
    return 1;
}

/** Changes the format of the matrix
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       MATRIX *matrix: the matrix that will have its format changed
*       MATRIX_FORMAT format_new: the new format for the matrix
*
*   return:
*       1 if executed successfully, 0 if not and fills the err_msg in SMOPS_CTX
*/
int MATRIX_change_format(SMOPS_CTX *ctx, MATRIX *matrix, MATRIX_FORMAT format_new)
{
    MATRIX_free_data(matrix);
    return MATRIX_set_format(ctx, matrix, format_new);
}

/** Creates a Matrix for storing the data of a Sparse Matrix
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*
*   return:
*       return an empty MATRIX with some values initialized by the SMOPS_CTX
*       returns NULL if an error has occurred and fills err_msg in SMOPS_CTX
*/
MATRIX *MATRIX_new(SMOPS_CTX *ctx)
{
    MATRIX *matrix = (MATRIX *)malloc(sizeof(MATRIX));
    if(matrix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for matrix");
        return NULL;
    }

    MATRIX_FORMAT OPERATION_FORMATS[] = { NONE, CSR, COO, CSR, COO, CSR };
    if(MATRIX_set_format(ctx,
            matrix,
            (MATRIX_FORMAT) OPERATION_FORMATS[ctx->operation]) == 0){
        free(matrix);
        return NULL;
    }

    matrix->type = UNDEFINED;
    return matrix;
}
