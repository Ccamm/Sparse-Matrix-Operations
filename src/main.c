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

int parse_opts(SMOPS_CTX *ctx, FILENAMES *filenames, float *sm_arg, int argc, char **argv)
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

int main(int argc, char **argv)
{
    int op_flag;
    float sm_arg;
    SMOPS_CTX *ctx = SMOPS_CTX_new();
    FILENAMES filenames;

    if(parse_opts(ctx, &filenames, &sm_arg, argc, argv) == 0) {
        SMOPS_CTX_print_err(ctx);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(filenames.file_name1 == NULL) {
        SMOPS_CTX_fill_err_msg(ctx, "no file provided as input");
        SMOPS_CTX_print_err(ctx);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(SMOPS_CTX_set_log_name_prefix(ctx, LOGPREFIX) == 0) {
        SMOPS_CTX_print_err(ctx);
        exit(EXIT_FAILURE);
    }

    printf("Prefix %s\n", ctx->log_prefix);

    MATRIX *a = MATRIX_init(ctx, filenames.file_name1);
    printf("Filename: %s\n", a->input_fn);
    op_flag = (int) SMOPS_CTX_get_operation(ctx);
    switch(op_flag) {
        case SCALAR_MULT:
            printf("Scalar Mult\n");
            break;
        case TRACE:
            printf("Trace\n");
            break;
        case ADD:
            if(filenames.file_name2 == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "no second file provided as input");
                SMOPS_CTX_print_err(ctx);
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            printf("Add\n");
            break;
        case TRANSPOSE:
            printf("Transpose\n");
            break;
        case MATRIX_MULT:
            if(filenames.file_name2 == NULL) {
                SMOPS_CTX_fill_err_msg(ctx, "no second file provided as input");
                SMOPS_CTX_print_err(ctx);
                exit(EXIT_FAILURE);
            }
            printf("Matrix Mult\n");
            break;
    }

    SMOPS_CTX_free(ctx);
    exit(EXIT_SUCCESS);
}
