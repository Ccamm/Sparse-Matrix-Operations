#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "smops.h"

#define BUFFER_SIZE 10
#define FLOAT_STR "float\0"
#define INT_STR "int\0"
#define READLINE if(fgets(buffer, BUFFER_SIZE, file) == NULL)

int read_data_line(SMOPS_CTX *ctx, MATRIX *matrix, char *dataline)
{
    int (* str_to_type)(char *) = NULL;
    switch(matrix->type) {
        case FLOAT:
            str_to_type = atof;
            break;
        case INT:
            str_to_type = atoi;
            break;
        default:
            SMOPS_CTX_fill_err_msg(ctx, "matrix type was incorrectly set");
            return 0;
    }
}
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
    FILE *file = fopen(filename, 'r');
    if(file == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to open file");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    READLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        close(file);
        return 0;
    }

    if(get_type(ctx, str) == UNDEFINED) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to get data type from input file");
        close(file);
        return 0;
    }

    READLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        close(file);
        return 0;
    }
    matrix->rows = atoi(buffer);
    READLINE {
        SMOPS_CTX_fill_err_msg(ctx, "failed to read file");
        close(file);
        return 0;
    }
    matrix->cols = atoi(buffer);
    matrix->size = matrix->rows * matrix->cols;
}
