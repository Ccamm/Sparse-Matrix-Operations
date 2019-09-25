#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "smops.h"

#define BUF_SIZE 30
#define TIME_FORMAT "%d%m%Y_%H%M_"
#define TIME_LEN 14
#define FILE_SUFFIX ".out\0"

char *get_log_filename(SMOPS_CTX *ctx, char *op_string)
{
    char *log_prefix = ctx->log_prefix;
    if(log_prefix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "log prefix has not been set!");
        return NULL;
    }
    char *filename = (char *)calloc(BUF_SIZE, sizeof(char));
    char *ptr = filename;
    strncpy(ptr, log_prefix, BUF_SIZE);
    ptr += strlen(log_prefix);

    time_t time_d;
    time(&time_d);
    struct tm *time_done = localtime( &time_d);
    strftime(ptr, BUF_SIZE - (ptr - filename), TIME_FORMAT, time_done);
    ptr += TIME_LEN;

    strncpy(ptr, op_string, BUF_SIZE - (ptr - filename));
    ptr += strlen(op_string);

    strncpy(ptr, FILE_SUFFIX, BUF_SIZE - (ptr - filename));
    return filename;
}

int display_results(SMOPS_CTX *ctx, FILE *fp, char *filename_a, char *filename_b, char *op_string)
{
    RESULT *result = ctx->result;
    if(result == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "result not set and cannot print");
        return 0;
    }
    fprintf(fp, "%s\n", op_string);
    if(filename_a != NULL) {
        fprintf(fp, "%s\n", filename_a);
    }
    if(filename_b != NULL) {
        fprintf(fp, "%s\n", filename_b);
    }
    fprintf(fp, "%d\n", ctx->thread_num);
    char *type_to_string[] = TYPE_MAP_STRING;
    switch(result->result_type) {
        case TRACE_SUM:
            fprintf(fp, "%s\n", type_to_string[result->type]);
            switch(result->type) {
                case INT:
                    fprintf(fp, "%d\n", result->result_data.trace.i);
                    break;
                case FLOAT:
                    fprintf(fp, "%f\n", result->result_data.trace.f);
                    break;
                case UNDEFINED:
                    SMOPS_CTX_fill_err_msg(ctx, "result has an UNDEFINED type");
                    return 0;
            }
            break;
        case DENSE_MATRIX:
            //RESULT_DATA dense_matrix = result->result_data;
            fprintf(fp, "%s\n", type_to_string[result->type]);
            fprintf(fp, "%d\n%d\n", result->rows, result->cols);
            int size = result->rows*result->cols;
            switch(result->type) {
                case INT:
                    for(int i = 0; i < size; i++) {
                        fprintf(fp, "%d ", result->result_data.matrix[i].i);
                    }
                    break;
                case FLOAT:
                    for(int i = 0; i < size; i++) {
                        fprintf(fp, "%f ", result->result_data.matrix[i].f);
                    }
                    break;
                case UNDEFINED:
                    SMOPS_CTX_fill_err_msg(ctx, "result has an UNDEFINED type");
                    return 0;
            }
            fprintf(fp, "\n");
            break;
    }
    fprintf(fp, "%f\n%f\n", ctx->time_load, ctx->time_op);
    return 1;
}

/** Presents the results of the operations by either printing or saving to a log file
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX where the result is saved
*       char *filename_a: the filename for the first matrix
*       char *filename_b: the filename for the second matrix
*
8   return:
*       1 if successfully executed, 0 otherwise filling the error message
*/
int SMOPS_RESULT_present(SMOPS_CTX *ctx, char *filename_a, char *filename_b)
{
    char *op_to_string[] = OP_MAP_STRING;
    char *op_string = op_to_string[ctx->operation];
    int result;
    char *output_filename;
    switch(ctx->log) {
            case 1:
                output_filename = get_log_filename(ctx, op_string);
                if(output_filename == NULL) {return 0;}
                FILE *output = fopen(output_filename, "w");
                if(output == NULL) {
                    SMOPS_CTX_fill_err_msg(ctx, "failed to create output file for logging");
                    return 0;
                }
                result = display_results(ctx, output, filename_a, filename_b, op_string);
                fclose(output);
                if(result == 0) {
                    free(output_filename);
                    return 0;
                }
                printf("%s: log file saved to %s\n", LIBNAME, output_filename);
                free(output_filename);
                return 1;
            default:
                result = display_results(ctx, stdout, filename_a, filename_b, op_string);
                if(result == 0) {return 0;}
                return 1;
    }
    SMOPS_CTX_fill_err_msg(ctx, "yeah nah shouldn't be here, the switch has goofed");
    return 0;
}

/** Frees the memory to a Result
*
*   parameters:
*       RESULT *result: the RESULT to be freed
*/
void SMOPS_RESULT_free(RESULT *result)
{
    if(result->result_type == DENSE_MATRIX) {
        if(result->result_data.matrix != NULL) {
            free(result->result_data.matrix);
        }
    }
    free(result);
}

/** Resets the result in the SMOPS_CTX if there is one already set
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX where the result is reset
*/
void reset_result(SMOPS_CTX *ctx)
{
    if(ctx->result != NULL) {
        SMOPS_RESULT_free(ctx->result);
        ctx->result = NULL;
    }
}

/** Saves the result of the operation of the form of a dense matrix to the SMOPS_CTX
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for errror handling and saving the result
*       MATRIX_DATA *dense_matrix: the dense matrix format of the result to be saved
*       TYPE type: the type the matrix is (either INT or FLOAT)
*       int rows: the number of rows the result has
*       int cols: the number of columns the result has
*
*   return:
*       1 if executed successfully, 0 otherwise filling error message
*/
int SMOPS_RESULT_save_matrix_result(SMOPS_CTX *ctx, MATRIX_DATA *dense_matrix, TYPE type,
                                    int rows, int cols)
{
    reset_result(ctx);
    RESULT *result = (RESULT *)calloc(1, sizeof(RESULT));
    if(result == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for result");
        return 0;
    }

    result->type = type;
    result->result_type = DENSE_MATRIX;
    result->result_data.matrix = dense_matrix;
    result->rows = rows;
    result->cols = cols;
    ctx->result = result;
    return 1;
}

/** Saves the result of the operation of the form of a trace sum to the SMOPS_CTX
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for errror handling and saving the result
*       MATRIX_DATA trace: the trace sum to be saved
*       TYPE type: the type the matrix is (either INT or FLOAT)
*       int rows: the number of rows the result has
*       int cols: the number of columns the result has
*
*   return:
*       1 if executed successfully, 0 otherwise filling error message
*/
int SMOPS_RESULT_save_trace_result(SMOPS_CTX *ctx, MATRIX_DATA trace, TYPE type)
{
    reset_result(ctx);
    RESULT *result = (RESULT *)calloc(1, sizeof(RESULT));
    if(result == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for result");
        return 0;
    }

    result->type = type;
    result->result_type = TRACE_SUM;
    result->result_data.trace = trace;
    result->rows = 0;
    result->cols = 0;
    ctx->result = result;
    return 1;
}

/** Saves the result of the operation of the form of a sparse matrix of format COO to the SMOPS_CTX
*
*   parameters:
*       SMOPS_CTX *ctx: the SMOPS_CTX for errror handling and saving the result
*       MATRIX *result: the result sparse matrix of format COO to save
*
*   return:
*       1 if executed successfully, 0 otherwise filling error message
*/
int SMOPS_RESULT_save_coo_matrix_result(SMOPS_CTX *ctx, MATRIX *result)
{
    if(result == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "no result matrix has been passed as result");
        return 0;
    }
    if(result->format != COO) {
        SMOPS_CTX_fill_err_msg(ctx, "result matrix does not have COO format or data");
        return 0;
    }
    int rows = result->rows;
    int cols = result->cols;
    int non_zero_size = result->non_zero_size;

    MATRIX_DATA *dense_matrix = COO_to_dense(ctx, result->coo_data, rows, cols, non_zero_size);
    if(dense_matrix == NULL) {return 0;}
    return SMOPS_RESULT_save_matrix_result(ctx, dense_matrix, result->type, rows, cols);
}
