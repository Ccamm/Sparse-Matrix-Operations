#define LIBNAME "SMOPS"
#define DEFAULT_THREAD_NUM 4
#define DEFAULT_LOG 0
#define ERR_MSG_BUFFER 100

/** Operations Supported By SMOPS
*
*/
enum ops {
    NO_OP=0,
    SCALAR_MULT=1,
    TRACE=2,
    ADD=3,
    TRANSPOSE=4,
    MATRIX_MULT=5
};
typedef enum ops OPERATION;

/** Struct for data used by the library
*   err_msg: error message if an error occurs while using library
*   err: 1 if an error has occurred, 0 otherwise
*   thread_num: number of threads to be used
*               if set to 1 will execute sequentially without OpenMP
*   log: 1 if results will be logged, 0 otherwise
*   operation: what sparse will be performed (required for loading matrices)
*/
struct smops_ctx {
    char *log_prefix;
    char *err_msg;
    int err;
    int thread_num;
    int log;
    OPERATION operation;
};
typedef struct smops_ctx SMOPS_CTX;

// SMOPS_CTX control routines defined in lib/smops_ctx.c
extern SMOPS_CTX *SMOPS_CTX_new();
extern void SMOPS_CTX_free(SMOPS_CTX *);
extern void SMOPS_CTX_fill_err_msg(SMOPS_CTX *, char *);
extern void SMOPS_CTX_print_err(SMOPS_CTX *);
extern int SMOPS_CTX_set_thread_num(SMOPS_CTX *, int);
extern int SMOPS_CTX_get_thread_num(SMOPS_CTX *);
extern int SMOPS_CTX_set_log(SMOPS_CTX *, int);
extern int SMOPS_CTX_get_log(SMOPS_CTX *);
extern void SMOPS_CTX_set_operation(SMOPS_CTX *, OPERATION);
extern OPERATION SMOPS_CTX_get_operation(SMOPS_CTX *);
extern int SMOPS_CTX_set_log_name_prefix(SMOPS_CTX *, char *);

enum mtype { INT, FLOAT, UNDEFINED };
typedef enum mtype MATRIX_TYPE;

enum mf { NONE=0, COO=1, CSR=2, CSC=3};
typedef enum mf MATRIX_FORMAT;

union md {
    int i;
    float f;
};
typedef union md MATRIX_DATA;

struct coo {
    int *coords_i;
    int *coords_j;
    MATRIX_DATA *values;
};
typedef struct coo COO_DATA;

struct csr {
    MATRIX_DATA *nnz;
    int *ia;
    int *ja;
};
typedef struct csr CSR_DATA;

struct csc {
    MATRIX_DATA *nnz;
    int *ia;
    int *ja;
};
typedef struct csr CSC_DATA;

struct m {
    char *input_fn;
    MATRIX_FORMAT format;
    MATRIX_TYPE type;
    COO_DATA *coo_data;
    CSR_DATA *csr_data;
    CSC_DATA *csc_data;
    int rows;
    int cols;
    int size;
    int non_zero_size;
};
typedef struct m MATRIX;

extern MATRIX *MATRIX_init(SMOPS_CTX *, char *);
extern MATRIX *MATRIX_new(SMOPS_CTX *);
extern int MATRIX_change_format(SMOPS_CTX *, MATRIX *, MATRIX_FORMAT);
extern int MATRIX_set_format(SMOPS_CTX *, MATRIX *, MATRIX_FORMAT);
extern void MATRIX_free(MATRIX *);
extern void MATRIX_free_data(MATRIX *);
