#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smops.h"

/** Creates a new SMOPS_CTX for storing data for the library.
*
*   return:
*       a pointer to the SMOPS_CTX
*/
SMOPS_CTX *SMOPS_CTX_new()
{
    SMOPS_CTX *ctx = (SMOPS_CTX *)malloc(sizeof(SMOPS_CTX));
    if(ctx == NULL) {
        fprintf(stderr, "%s: ERROR! failed to allocate memory for ctx\n", LIBNAME);
        exit(EXIT_FAILURE);
    }

    ctx->err_msg = (char *)malloc(sizeof(char)*ERR_MSG_BUFFER);
    if(ctx->err_msg == NULL) {
        fprintf(stderr, "%s: ERROR! failed to allocate memory for error message\n", LIBNAME);
        exit(EXIT_FAILURE);
    }

    ctx->thread_num = DEFAULT_THREAD_NUM;
    ctx->log = DEFAULT_LOG;
    ctx->err = 0;
    ctx->operation = NO_OP;
    ctx->log_prefix = NULL;
    ctx->time_load = 0;
    ctx->time_op = 0;
    ctx->result = NULL;
    return ctx;
}

/** Frees all data inside the SMOPS_CTX
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX to be freed
*/
void SMOPS_CTX_free(SMOPS_CTX *ctx)
{
    if(ctx->err_msg != NULL) free(ctx->err_msg);
    if(ctx->log_prefix != NULL) free(ctx->log_prefix);
    if(ctx->result != NULL) SMOPS_RESULT_free(ctx->result);
    free(ctx);
}

/** Fills the err_msg in SMOPS_CTX with an error message
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       err_msg: the error message to put into the SMOPS_CTX
*/
void SMOPS_CTX_fill_err_msg(SMOPS_CTX *ctx, char *err_msg)
{
    int err_msg_len = strlen(err_msg);
    if(err_msg_len + 1 > ERR_MSG_BUFFER) {
        fprintf(stderr, "%s: ERROR! error message is greater than %d chars long\n", LIBNAME, ERR_MSG_BUFFER);
        exit(EXIT_FAILURE);
    }
    ctx->err = 1;
    strncpy(ctx->err_msg, err_msg, err_msg_len);
    *(ctx->err_msg + err_msg_len) = '\0';
}

/** Print the error message stored in the SMOPS_CTX if an error has occurred
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*
*/
void SMOPS_CTX_print_err(SMOPS_CTX *ctx)
{
    if(ctx->err == 1) {
        printf("%s: ERROR! %s\n", LIBNAME, ctx->err_msg);
    }
}

/** Sets the number of threads to be used for calculations.
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       int thread_num: the number of threads
*
*   return:
*       1 if valid thread number has been put into in (thread_num > 0)
*       0 otherwise and fills err_msg
*/
int SMOPS_CTX_set_thread_num(SMOPS_CTX *ctx, int thread_num)
{
    if(thread_num <= 0) {
        SMOPS_CTX_fill_err_msg(ctx, "thread_num is invalid (thread_num <= 0)");
        return 0;
    }
    ctx->thread_num = thread_num;
    return 1;
}

/** Gets the number of threads that should be used.
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*
*   returns:
*       the number of threads to be used
*/
int SMOPS_CTX_get_thread_num(SMOPS_CTX *ctx)
{
    return ctx->thread_num;
}

/** Set if results should be logged to file
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       int log: 1 if results should be saved, 0 otherwise
*
*   return:
*       1 if log is either 1 or 0, 0 otherwise and fills err_msg
*/
int SMOPS_CTX_set_log(SMOPS_CTX *ctx, int log)
{
    if(log < 0 && log > 1) {
        SMOPS_CTX_fill_err_msg(ctx, "log is invalid (can only be 1 or 0)");
        return 0;
    }
    ctx->log = log;
    return 1;
}

/** Gets if the program should save results to file
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*
*   returns:
*       1 if the program should save results to file, 0 if not
*/
int SMOPS_CTX_get_log(SMOPS_CTX *ctx)
{
    return ctx->log;
}

/** Sets the operation that will be performed
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       OPERATION op: the operation that will be performed
*/
void SMOPS_CTX_set_operation(SMOPS_CTX *ctx, OPERATION op)
{
    ctx->operation = op;
}

/** Gets the operation that will be performed
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*
*   return:
*       the OPERATION that will be performed
*/
OPERATION SMOPS_CTX_get_operation(SMOPS_CTX *ctx)
{
    return ctx->operation;
}

/** Sets the prefix of the log filename
*
*   parameters:
*       SMOPS_CTX *ctx: a pointer to the SMOPS_CTX
*       char *prefix: the prefix of the log filename
*
*   return:
*       1 if successfully executed, 0 otherwise filling the err_msg
*/
int SMOPS_CTX_set_log_name_prefix(SMOPS_CTX *ctx, char *prefix)
{
    int prefix_len = strlen(prefix);
    ctx->log_prefix = (char *)malloc(sizeof(char)*(prefix_len+1));
    if(ctx->log_prefix == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "failed to allocate memory for log file name prefix");
        return 0;
    }
    strncpy(ctx->log_prefix, prefix, prefix_len);
    ctx->log_prefix[prefix_len] = '\0';
    return 1;
}
