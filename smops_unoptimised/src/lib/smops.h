#define LIBNAME "SMOPS"
#define DEFAULT_THREAD_NUM 4
#define DEFAULT_LOG 0
#define ERR_MSG_BUFFER 100
#define OP_MAP_FORMAT { NONE, COO, COO, CSR, COO, CSR }
#define BILLION 1000000000.0
#define OP_MAP_STRING { "noop\0", "sm\0", "tr\0", "ad\0", "ts\0", "mm\0" }
#define TYPE_MAP_STRING { "int\0", "float\0", "undefined\0" }

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

enum mtype { INT=0, FLOAT=1, UNDEFINED=2 };
typedef enum mtype TYPE;

enum mf { NONE=0, COO=1, CSR=2, CSC=3};
typedef enum mf MATRIX_FORMAT;

union md {
    int i;
    double f;
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

union result_data {
    MATRIX_DATA *matrix;
    MATRIX_DATA trace;
};
typedef union result_data RESULT_DATA;

enum result_type { DENSE_MATRIX, TRACE_SUM };
typedef enum result_type RESULT_TYPE;

struct result {
    TYPE type;
    RESULT_TYPE result_type;
    RESULT_DATA result_data;
    int rows;
    int cols;
};
typedef struct result RESULT;

/** Struct for data used by the library
*   err_msg: error message if an error occurs while using library
*   err: 1 if an error has occurred, 0 otherwise
*   thread_num: number of threads to be used
*               if set to 1 will execute sequentially without OpenMP
*   log: 1 if results will be logged to file, anything else prints results
*   operation: what sparse will be performed (required for loading matrices)
*/
struct smops_ctx {
    char *log_prefix;
    char *err_msg;
    int err;
    int thread_num;
    int log;
    double time_load;
    double time_op;
    OPERATION operation;
    RESULT *result;
};
typedef struct smops_ctx SMOPS_CTX;

//TO DO! Create a union for storing result, either pointer to dense format or single result
//For results you can either print to screen or temporarily change stdout to file

struct m {
    MATRIX_FORMAT format;
    TYPE type;
    COO_DATA *coo_data;
    CSR_DATA *csr_data;
    CSC_DATA *csc_data;
    int rows;
    int cols;
    int size;
    int non_zero_size;
};
typedef struct m MATRIX;

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

extern int SMOPS_RESULT_save_trace_result(SMOPS_CTX *, MATRIX_DATA, TYPE);
extern int SMOPS_RESULT_save_coo_matrix_result(SMOPS_CTX *, MATRIX *);
extern int SMOPS_RESULT_save_matrix_result(SMOPS_CTX *, MATRIX_DATA *, TYPE, int, int);
extern void SMOPS_RESULT_free(RESULT *);
extern int SMOPS_RESULT_present(SMOPS_CTX *, char *, char *);

extern MATRIX *MATRIX_new(SMOPS_CTX *);
extern int MATRIX_change_format(SMOPS_CTX *, MATRIX *, MATRIX_FORMAT);
extern int MATRIX_set_format(SMOPS_CTX *, MATRIX *, MATRIX_FORMAT);
extern void MATRIX_free(MATRIX *);
extern void MATRIX_free_data(MATRIX *);
extern int MATRIX_preload_type(SMOPS_CTX *, MATRIX *, char *, MATRIX *, char *);
extern int MATRIX_load(SMOPS_CTX *, MATRIX *, char *);
extern int MATRIX_set_properties(SMOPS_CTX *, MATRIX *, MATRIX_FORMAT, TYPE, int, int, int);

extern COO_DATA *COO_new(SMOPS_CTX *);
extern CSR_DATA *CSR_new(SMOPS_CTX *);
extern CSC_DATA *CSC_new(SMOPS_CTX *);
extern void COO_free(COO_DATA *);
extern void CSR_free(CSR_DATA *);
extern void CSC_free(CSR_DATA *);
extern void COO_sort_row_order(COO_DATA *, int);
extern void COO_sort_col_order(COO_DATA *, int);
extern MATRIX_DATA *COO_to_dense(SMOPS_CTX *, COO_DATA *, int, int, int);

extern int OPS_check_format(SMOPS_CTX *, MATRIX *, OPERATION op, MATRIX_FORMAT);

extern int MATRIX_OP_trace(SMOPS_CTX *, MATRIX_DATA *, MATRIX *);
extern int MATRIX_OP_transpose(SMOPS_CTX *, MATRIX *, MATRIX *);
extern int MATRIX_OP_scalar_multiplication(SMOPS_CTX *, MATRIX *, MATRIX *, double);
extern int MATRIX_OP_addition(SMOPS_CTX *, MATRIX *, MATRIX *);
extern int MATRIX_OP_multiplication(SMOPS_CTX *, MATRIX *, MATRIX *);
