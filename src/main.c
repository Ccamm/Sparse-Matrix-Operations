#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "smopslib.h"

#define OPTLIST "t:lf:"

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

int parse_opts(SMOPS_CTX *ctx, FILENAMES *filenames int *op_flag, int argc, char **argv)
{
    int opt, index;
    int op_flag_temp = NO_OP;
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
                    SMOPS_CTX_print_err(ctx);
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'l':
                SMOPS_CTX_set_log(ctx, 1);
                break;
            case 'f':
                index = optind;
                if(index >= argc || argv[index] == '-') {
                    fprintf("%s: no file name specified after -f option");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                filenames->file_name1 = argv[index];
                index++;
                if(index < argc && argv[index] != '-') {
                    filenames->file_name2 = argv[index];
                }
                break;
        }
    }

    if (op_flag_temp == NO_OP) {
        fprintf(stderr, "%s: no matrix operation provided\n", argv[0]);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    *op_flag = op_flag_temp;
    return 1;
}

int main(int argc, char **argv)
{
    int op_flag;
    SMOPS_CTX *ctx = SMOPS_CTX_new();
    FILENAMES filenames;

    if(parse_opts(ctx, &op_flag, argc, argv) == 0) {
        SMOPS_CTX_print_err(ctx);
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}
