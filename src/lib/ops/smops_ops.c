#include <stdlib.h>

#include "../smops.h"

int OPS_check_format(SMOPS_CTX *ctx, MATRIX *matrix, OPERATION op, MATRIX_FORMAT override)
{
    MATRIX_FORMAT OPERATION_FORMATS[] = OP_MAP_FORMAT;
    if(matrix->format != OPERATION_FORMATS[op] && matrix->format != override) {
        SMOPS_CTX_fill_err_msg(ctx, "incorrect format set for matrix operation");
        return 0;
    }
    if(matrix->format == override && override == NONE) {
        SMOPS_CTX_fill_err_msg(ctx, "matrix has format of NONE which is not possible");
        return 0;
    }
    return 1;
}
