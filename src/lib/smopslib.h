#define LIBNAME "SMOPS"
#define DEFAULT_THREAD_NUM 4
#define DEFAULT_LOG 0
#define ERR_MSG_BUFFER 100

enum OPERATIONS {
    NO_OP,
    SCALAR_MULT,
    TRACE,
    ADD,
    TRANSPOSE,
    MATRIX_MULT
};

/** Struct for data used by the library
*   err_msg: error message if an error occurs while using library
*   err: 1 if an error has occurred, 0 otherwise
*   thread_num: number of threads to be used
*               if set to 1 will execute sequentially without OpenMP
*   log: 1 if results will be logged, 0 otherwise
*/
struct smops_ctx {
    char *err_msg;
    int err;
    int thread_num;
    int log;
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
