#include <stdlib.h>
#include <omp.h>

#include "smops.h"

MATRIX_DATA *COO_to_dense(SMOPS_CTX *ctx, COO_DATA *coo_data, int rows, int cols, int non_zero_size)
{
    if(coo_data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "tried to convert empty COO_DATA to a dense matrix");
        return NULL;
    }
    MATRIX_DATA *dense_matrix = (MATRIX_DATA *)calloc(rows*cols, sizeof(MATRIX_DATA));
    if(dense_matrix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for dense matrix from COO_DATA");
        return NULL;
    }

    int i, index;
    switch(ctx->thread_num) {
        case 1:
            for(i = 0; i < non_zero_size; i++) {
                index = coo_data->coords_i[i]*rows + coo_data->coords_j[i];
                dense_matrix[index] = coo_data->values[i];
            }
            break;
        default:
            #pragma omp parallel num_threads(ctx->thread_num) private(i, index)
            {
                #pragma omp for
                for(i = 0; i < non_zero_size; i++) {
                    index = coo_data->coords_i[i]*rows + coo_data->coords_j[i];
                    dense_matrix[index] = coo_data->values[i];
                }
            }
            break;
    }
    return dense_matrix;
}
/** Initialises the memory for CSC_DATA
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX for error handling
*
*   return:
*       a pointer to the allocated memory for the COO_DATA
*       returns NULL if an error has occurred
*/
CSC_DATA *CSC_new(SMOPS_CTX *ctx)
{
    CSC_DATA *data = (CSC_DATA *)malloc(sizeof(CSC_DATA));
    if(data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for CSC_DATA for matrix");
        return NULL;
    }
    data->nnz = NULL;
    data->ia = NULL;
    data->ja = NULL;
    return data;
}

/** Initialises the memory for CSR_DATA
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX for error handling
*
*   return:
*       a pointer to the allocated memory for the CSR_DATA
*       returns NULL if an error has occurred
*/
CSR_DATA *CSR_new(SMOPS_CTX *ctx)
{
    CSR_DATA *data = (CSR_DATA *)malloc(sizeof(CSR_DATA));
    if(data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for CSR_DATA for matrix");
        return NULL;
    }
    data->nnz = NULL;
    data->ia = NULL;
    data->ja = NULL;
    return data;
}

/** Initialises the memory for COO_DATA
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX for error handling
*
*   return:
*       a pointer to the allocated memory for the COO_DATA
*       returns NULL if an error has occurred
*/
COO_DATA *COO_new(SMOPS_CTX *ctx)
{
    COO_DATA *data = (COO_DATA *)malloc(sizeof(COO_DATA));
    if(data == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for COO_DATA for matrix");
        return NULL;
    }
    data->coords_i = NULL;
    data->coords_j = NULL;
    data->values = NULL;
    return data;
}

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

/** Swaps the position of an element at index a with index b in the COO_DATA
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA where the elements are swapped around.
*       int a: the index of the element to be swapped with the element at index b
*       int b: the index of the element to be swapped with the element at index a
*/
void coo_swap_data(COO_DATA *coo_data, int a, int b)
{
    int temp_ai, temp_aj;
    MATRIX_DATA temp_av;

    temp_ai = coo_data->coords_i[a];
    temp_aj = coo_data->coords_j[a];
    temp_av = coo_data->values[a];

    coo_data->coords_i[a] = coo_data->coords_i[b];
    coo_data->coords_j[a] = coo_data->coords_j[b];
    coo_data->values[a] = coo_data->values[b];

    coo_data->coords_i[b] = temp_ai;
    coo_data->coords_j[b] = temp_aj;
    coo_data->values[b] = temp_av;
}

/** The partition routine used by coo_quicksort.
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA to be sorted
*       int *sort: the array to sort by
*       int p: lower bound of the quicksort
*       int q: upper bound of the quicksort
*
*   return:
*       the index of where the temp value has been sorted too
*/
int coo_quicksort_partition(COO_DATA *coo_data, int *sort, int p, int q)
{
    int temp = sort[q];
    int pivot = p-1;

    for(int i = p; i <= q - 1; i++) {
        if(sort[i] <= temp) {
            pivot++;
            coo_swap_data(coo_data, pivot, i);
        }
    }
    coo_swap_data(coo_data, pivot+1, q);
    return pivot+1;
}

/** Quicksorts the COO_DATA
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA to be sorted
*       int *sort: the array to sort by
*       int p: lower bound of the quicksort
*       int q: upper bound of the quicksort
*/
void coo_quicksort(COO_DATA *coo_data, int *sort, int p, int q)
{
    if(p < q) {
        int part = coo_quicksort_partition(coo_data, sort, p, q);
        coo_quicksort(coo_data, sort, p, part - 1);
        coo_quicksort(coo_data, sort, part + 1, q);
    }
}

/** Sorts the COO_DATA by first sorting by array_a then sorting array_b for same
*   elements in array_a.
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA to be sorted
*       int *array_a: the array to be used first to sort
*       int *array_b: the array to be used to sort the second time
*       int non_zero_size: the number of non zero elements stored in COO_DATA
*/
void coo_sort_order(COO_DATA *coo_data, int *array_a, int *array_b, int non_zero_size)
{
    coo_quicksort(coo_data, array_a, 0, non_zero_size - 1);

    int p = 0;
    int q = 0;

    for(int i=1; i < non_zero_size; i++) {
        if(array_a[i-1] != array_a[i]) {
            coo_quicksort(coo_data, array_b, p, q);
            p = i;
            q = i;
        } else {
            q++;
        }
    }
}

/** Sort the COO data structure in row major order
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA to be sorted
*       int non_zero_size: the number of non zero elements stored in COO_DATA
*/
void COO_sort_row_order(COO_DATA *coo_data, int non_zero_size)
{
    coo_sort_order(coo_data, coo_data->coords_i, coo_data->coords_j, non_zero_size);
}

/** Sort the COO data structure in column major order
*
*   parameters:
*       COO_DATA *coo_data: the COO_DATA to be sorted
*       int non_zero_size: the number of non zero elements stored in COO_DATA
*/
void COO_sort_col_order(COO_DATA *coo_data, int non_zero_size)
{
    coo_sort_order(coo_data, coo_data->coords_j, coo_data->coords_i, non_zero_size);
}
