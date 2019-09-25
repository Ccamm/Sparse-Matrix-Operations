#include <stdlib.h>
#include "smops.h"

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

    matrix->coo_data = COO_new(ctx);
    if(matrix->coo_data == NULL) {return 0;}

    switch(matrix->format){
        case CSR:
            matrix->csr_data = CSR_new(ctx);
            if(matrix->csr_data == NULL) {return 0;}
            break;
        case COO:
            break;
        case CSC:
            matrix->csc_data = CSC_new(ctx);
            if(matrix->csc_data == NULL) {return 0;}
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

/** Changes the properties of a MATRIX
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for handling errors
*       MATRIX *matrix: the matrix which its properties will be changed
*       MATRIX_FORMAT format: the new format for the matrix
*       TYPE type: the new type for the matrix
*       int rows: the new number of rows for the matrix
*       int cols: the new number of cols for the matrix
*       int non_zero_size: the number of non zero elements to be set for matrix
*
*   return:
*       1 if successfully executed, 0 otherwise filling error message
*/
int MATRIX_set_properties(SMOPS_CTX *ctx, MATRIX *matrix, MATRIX_FORMAT format,
                            TYPE type, int rows, int cols, int non_zero_size)
{
    if(matrix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "a null matrix was used a parameter for MATRIX_set_properties");
        return 0;
    }
    matrix->type = type;
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->size = rows*cols;
    matrix->non_zero_size = non_zero_size;
    return MATRIX_change_format(ctx, matrix, format);
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

    MATRIX_FORMAT OPERATION_FORMATS[] = OP_MAP_FORMAT;
    if(MATRIX_set_format(ctx,
            matrix,
            (MATRIX_FORMAT) OPERATION_FORMATS[ctx->operation]) == 0){
        free(matrix);
        return NULL;
    }

    matrix->type = UNDEFINED;
    return matrix;
}
