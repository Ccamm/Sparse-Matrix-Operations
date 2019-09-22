#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "lib/smops.h"

#define OPTLIST "t:lf:"
#define LOGPREFIX "21955725_\0"

struct filenames {
    char *file_name1;
    char *file_name2;
};
typedef struct filenames FILENAMES;

void usage(char *progam_name)
{
    printf("Usage for %s\n", progam_name);
    printf("%s [matrix operation] [options] -f [file] [optional file]\n\n",
        progam_name);
    printf("matrix operations:\n");
    printf("\t--sm [a]: Scalar Multiplication of the input matrix by a\n");
    printf("\t--tr: Calculate the Trace of the input matrix\n");
    printf("\t--ad: Add two matrices together specifed by the -f option\n");
    printf("\t--ts: Calculate the Transpose of the input matrix\n");
    printf("\t--mm: Multiply two matrices specified by the -f option\n\n");
    printf("options:\n");
    printf("\t-t [number of threads]: How many threads should be used\n");
    printf("\t-l: Results will be logged to file\n\n");
    printf("matrix input: -f [file] [optional file]\n");
    printf("\tfile: file name of the input matrix\n");
    printf("\toptional file: file name of the other input matrix for ad and mm\n");
}

int parse_opts(SMOPS_CTX *ctx, FILENAMES *filenames, double *sm_arg, int argc, char **argv)
{
    int opt, index;
    int op_flag_temp = NO_OP;
    filenames->file_name1 = NULL;
    filenames->file_name2 = NULL;

    struct option long_optlist[] = {
        {"sm", required_argument, &op_flag_temp, SCALAR_MULT},
        {"tr", no_argument, &op_flag_temp, TRACE},
        {"ad", no_argument, &op_flag_temp, ADD},
        {"ts", no_argument, &op_flag_temp, TRANSPOSE},
        {"mm", no_argument, &op_flag_temp, MATRIX_MULT},
        {   0, no_argument, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, OPTLIST, long_optlist, NULL)) != -1) {
        switch (opt) {
            case 't':
                if(SMOPS_CTX_set_thread_num(ctx, atoi(optarg)) == 0) {
                    return 0;
                }
                break;
            case 'l':
                SMOPS_CTX_set_log(ctx, 1);
                break;
            case 'f':
                filenames->file_name1 = optarg;
                index = optind;
                if(index < argc && *argv[index] != '-') {
                    filenames->file_name2 = argv[index];
                }
                break;
            case 0:
                switch (op_flag_temp) {
                    case SCALAR_MULT:
                        *sm_arg = atof(optarg);
                        break;
                }
        }
    }

    if (op_flag_temp == NO_OP) {
        SMOPS_CTX_fill_err_msg(ctx, "no matrix operation provided");
        return 0;
    }

    SMOPS_CTX_set_operation(ctx, (OPERATION) op_flag_temp);
    return 1;
}

void smops_exit(SMOPS_CTX *ctx, MATRIX *a, MATRIX *b, MATRIX *c)
{
    if(ctx != NULL) {
        SMOPS_CTX_print_err(ctx);
        SMOPS_CTX_free(ctx);
    }
    if(a != NULL) MATRIX_free(a);
    if(b != NULL) MATRIX_free(b);
    if(c != NULL) MATRIX_free(c);;
}

int main(int argc, char **argv)
{
    double sm_arg;
    SMOPS_CTX *ctx = SMOPS_CTX_new();
    if(ctx == NULL) {
        fprintf(stderr, "Could not make SMOPS_CTX for controlling operation\n");
        exit(EXIT_FAILURE);
    }
    FILENAMES filenames;

    if(parse_opts(ctx, &filenames, &sm_arg, argc, argv) == 0) {
        smops_exit(ctx, NULL, NULL, NULL);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(filenames.file_name1 == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "no file provided as input");
        smops_exit(ctx, NULL, NULL, NULL);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(SMOPS_CTX_set_log_name_prefix(ctx, LOGPREFIX) == 0) {
        smops_exit(ctx, NULL, NULL, NULL);
        exit(EXIT_FAILURE);
    }

    MATRIX *a = MATRIX_new(ctx);
    if(a == NULL) {
        smops_exit(ctx, NULL, NULL, NULL);
        exit(EXIT_FAILURE);
    }

    MATRIX *b = NULL;

    MATRIX *result = MATRIX_new(ctx);
    if(result == NULL) {
        smops_exit(ctx, a, b, result);
        exit(EXIT_FAILURE);
    }

    switch(SMOPS_CTX_get_operation(ctx)) {
        case SCALAR_MULT:
            printf("Scalar Mult\n");
            if(MATRIX_load(ctx, a, filenames.file_name1) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            MATRIX_OP_scalar_multiplication(ctx, result, a, sm_arg);
            break;
        case TRACE:
            printf("Trace\n");
            if(MATRIX_load(ctx, a, filenames.file_name1) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            MATRIX_DATA result_num;
            MATRIX_OP_trace(ctx, &result_num, a);
            break;
        case ADD:
            if(filenames.file_name2 == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "no second file provided as input");
                smops_exit(ctx, a, b, result);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if((b = MATRIX_new(ctx)) == NULL){
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }

            printf("Add\n");
            if(MATRIX_preload_type(ctx, a, filenames.file_name1, b, filenames.file_name2) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            if(MATRIX_load(ctx, a, filenames.file_name1) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            if(MATRIX_load(ctx, b, filenames.file_name2) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            MATRIX_OP_addition(ctx, a, b);
            break;
        case TRANSPOSE:
            printf("Transpose\n");
            if(MATRIX_load(ctx, a, filenames.file_name1) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            MATRIX_OP_transpose(ctx, result, a);
            break;
        case MATRIX_MULT:
            printf("Matrix Mult\n");
            if(filenames.file_name2 == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "no second file provided as input");
                smops_exit(ctx, a, b, result);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            if((b = MATRIX_new(ctx)) == NULL){
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }

            if(MATRIX_preload_type(ctx, a, filenames.file_name1, b, filenames.file_name2) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }

            if(MATRIX_change_format(ctx, b, CSC) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            if(MATRIX_load(ctx, b, filenames.file_name2) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }

            if(MATRIX_load(ctx, a, filenames.file_name1) == 0) {
                smops_exit(ctx, a, b, result);
                exit(EXIT_FAILURE);
            }
            MATRIX_OP_multiplication(ctx, a, b);
            break;
        default:
            break;
    }
    SMOPS_RESULT_present(ctx, filenames.file_name1, filenames.file_name2);
    smops_exit(ctx, a, b, result);
    exit(EXIT_SUCCESS);
}
