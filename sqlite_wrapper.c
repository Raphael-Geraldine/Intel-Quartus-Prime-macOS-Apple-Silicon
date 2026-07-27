#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <sqlite3.h>

// Macros limpas: o prefixo sqlite3_ é injetado automaticamente
#define ALTR_WRAP_VOID(name, params, args) \
    void altr_sqlite3_##name params { \
        sqlite3_##name args; \
    }

#define ALTR_WRAP_NONVOID(ret_type, name, params, args) \
    ret_type altr_sqlite3_##name params { \
        return sqlite3_##name args; \
    }

// --- Gerenciamento e Ciclo de Vida ---
ALTR_WRAP_NONVOID(int, initialize, (void), ())
ALTR_WRAP_NONVOID(int, shutdown, (void), ())
ALTR_WRAP_NONVOID(const char *, libversion, (void), ())
ALTR_WRAP_NONVOID(int, complete, (const char *sql), (sql))

// --- Conexão e Banco ---
ALTR_WRAP_NONVOID(int, open_v2, (const char *filename, sqlite3 **ppDb, int flags, const char *zVfs), (filename, ppDb, flags, zVfs))
ALTR_WRAP_NONVOID(int, close, (sqlite3 *db), (db))
ALTR_WRAP_NONVOID(int, busy_handler, (sqlite3 *db, int (*xPy)(void*,int), void *pArg), (db, xPy, pArg))
ALTR_WRAP_NONVOID(int, busy_timeout, (sqlite3 *db, int ms), (db, ms))
ALTR_WRAP_VOID(interrupt, (sqlite3 *db), (db))
ALTR_WRAP_NONVOID(sqlite3_int64, last_insert_rowid, (sqlite3 *db), (db))
ALTR_WRAP_NONVOID(int, changes, (sqlite3 *db), (db))
ALTR_WRAP_NONVOID(int, db_status, (sqlite3 *db, int op, int *pCur, int *pMax, int resetFlg), (db, op, pCur, pMax, resetFlg))
ALTR_WRAP_NONVOID(int, enable_shared_cache, (int enable), (enable))

// --- Erros ---
ALTR_WRAP_NONVOID(const char *, errmsg, (sqlite3 *db), (db))
ALTR_WRAP_NONVOID(int, errcode, (sqlite3 *db), (db))

// --- Execução Directa ---
ALTR_WRAP_NONVOID(int, exec, (sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg), (db, sql, callback, arg, errmsg))

// --- Prepared Statements ---
ALTR_WRAP_NONVOID(int, prepare_v2, (sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail), (db, zSql, nByte, ppStmt, pzTail))
ALTR_WRAP_NONVOID(int, prepare16_v2, (sqlite3 *db, const void *zSql, int nByte, sqlite3_stmt **ppStmt, const void **pzTail), (db, zSql, nByte, ppStmt, pzTail))
ALTR_WRAP_NONVOID(int, step, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(int, reset, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(int, finalize, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(int, clear_bindings, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(int, data_count, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(int, stmt_status, (sqlite3_stmt *pStmt, int op, int resetFlg), (pStmt, op, resetFlg))

// --- Binds ---
ALTR_WRAP_NONVOID(int, bind_blob, (sqlite3_stmt *p, int i, const void *z, int n, void(*x)(void*)), (p, i, z, n, x))
ALTR_WRAP_NONVOID(int, bind_double, (sqlite3_stmt *p, int i, double r), (p, i, r))
ALTR_WRAP_NONVOID(int, bind_int, (sqlite3_stmt *p, int i, int iVal), (p, i, iVal))
ALTR_WRAP_NONVOID(int, bind_int64, (sqlite3_stmt *p, int i, sqlite3_int64 iVal), (p, i, iVal))
ALTR_WRAP_NONVOID(int, bind_null, (sqlite3_stmt *p, int i), (p, i))
ALTR_WRAP_NONVOID(int, bind_parameter_count, (sqlite3_stmt *p), (p))
ALTR_WRAP_NONVOID(int, bind_parameter_index, (sqlite3_stmt *p, const char *z), (p, z))
ALTR_WRAP_NONVOID(const char *, bind_parameter_name, (sqlite3_stmt *p, int i), (p, i))
ALTR_WRAP_NONVOID(int, bind_text, (sqlite3_stmt *p, int i, const char *z, int n, void(*x)(void*)), (p, i, z, n, x))
ALTR_WRAP_NONVOID(int, bind_text16, (sqlite3_stmt *p, int i, const void *z, int n, void(*x)(void*)), (p, i, z, n, x))
ALTR_WRAP_NONVOID(int, bind_zeroblob, (sqlite3_stmt *p, int i, int n), (p, i, n))

// --- Leitura de Colunas e Valores ---
ALTR_WRAP_NONVOID(int, column_count, (sqlite3_stmt *pStmt), (pStmt))
ALTR_WRAP_NONVOID(const char *, column_name, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const void *, column_name16, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const char *, column_decltype, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const void *, column_decltype16, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const void *, column_database_name16, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const void *, column_table_name16, (sqlite3_stmt *pStmt, int N), (pStmt, N))
ALTR_WRAP_NONVOID(const void *, column_origin_name16, (sqlite3_stmt *pStmt, int N), (pStmt, N))

ALTR_WRAP_NONVOID(const void *, column_blob, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(int, column_bytes, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(int, column_bytes16, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(double, column_double, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(int, column_int, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(sqlite3_int64, column_int64, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(const unsigned char *, column_text, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(const void *, column_text16, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(int, column_type, (sqlite3_stmt *pStmt, int iCol), (pStmt, iCol))
ALTR_WRAP_NONVOID(const unsigned char *, value_text, (sqlite3_value *pVal), (pVal))

// --- Memória e Formatação ---
ALTR_WRAP_NONVOID(void *, malloc, (int n), (n))
ALTR_WRAP_VOID(free, (void *p), (p))
ALTR_WRAP_NONVOID(int, status, (int op, int *pCurrent, int *pHighwater, int resetFlag), (op, pCurrent, pHighwater, resetFlag))

char *altr_sqlite3_mprintf(const char *zFormat, ...) {
    va_list ap;
    va_start(ap, zFormat);
    char *res = sqlite3_vmprintf(zFormat, ap);
    va_end(ap);
    return res;
}

// --- Blobs & Backup ---
ALTR_WRAP_NONVOID(int, blob_open, (sqlite3* db, const char *zDb, const char *zTable, const char *zColumn, sqlite3_int64 iRow, int flags, sqlite3_blob **ppBlob), (db, zDb, zTable, zColumn, iRow, flags, ppBlob))
ALTR_WRAP_NONVOID(int, blob_close, (sqlite3_blob *pBlob), (pBlob))
ALTR_WRAP_NONVOID(int, blob_bytes, (sqlite3_blob *pBlob), (pBlob))
ALTR_WRAP_NONVOID(int, blob_read, (sqlite3_blob *pBlob, void *z, int n, int iOffset), (pBlob, z, n, iOffset))
ALTR_WRAP_NONVOID(int, blob_write, (sqlite3_blob *pBlob, const void *z, int n, int iOffset), (pBlob, z, n, iOffset))

ALTR_WRAP_NONVOID(sqlite3_backup *, backup_init, (sqlite3 *pDest, const char *zDestName, sqlite3 *pSource, const char *zSourceName), (pDest, zDestName, pSource, zSourceName))
ALTR_WRAP_NONVOID(int, backup_step, (sqlite3_backup *p, int nPage), (p, nPage))
ALTR_WRAP_NONVOID(int, backup_finish, (sqlite3_backup *p), (p))
ALTR_WRAP_NONVOID(int, backup_remaining, (sqlite3_backup *p), (p))
ALTR_WRAP_NONVOID(int, backup_pagecount, (sqlite3_backup *p), (p))

// --- Funções Customizadas & Callbacks ---
ALTR_WRAP_NONVOID(int, create_function, (sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, void (*xFunc)(sqlite3_context*,int,sqlite3_value**), void (*xStep)(sqlite3_context*,int,sqlite3_value**), void (*xFinal)(sqlite3_context*)), (db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal))
ALTR_WRAP_NONVOID(int, aggregate_count, (sqlite3_context *pCtx), (pCtx))
ALTR_WRAP_NONVOID(void *, user_data, (sqlite3_context *pCtx), (pCtx))
ALTR_WRAP_VOID(result_blob, (sqlite3_context *pCtx, const void *z, int n, void (*x)(void*)), (pCtx, z, n, x))
ALTR_WRAP_VOID(result_double, (sqlite3_context *pCtx, double r), (pCtx, r))
ALTR_WRAP_VOID(result_error, (sqlite3_context *pCtx, const char *z, int n), (pCtx, z, n))
ALTR_WRAP_VOID(result_error16, (sqlite3_context *pCtx, const void *z, int n), (pCtx, z, n))
ALTR_WRAP_VOID(result_int, (sqlite3_context *pCtx, int i), (pCtx, i))
ALTR_WRAP_VOID(result_null, (sqlite3_context *pCtx), (pCtx))
ALTR_WRAP_VOID(result_text16, (sqlite3_context *pCtx, const void *z, int n, void (*x)(void*)), (pCtx, z, n, x))
ALTR_WRAP_VOID(result_zeroblob, (sqlite3_context *pCtx, int n), (pCtx, n))

ALTR_WRAP_NONVOID(void *, trace, (sqlite3 *db, void (*xTrace)(void*,const char*), void *pArg), (db, xTrace, pArg))
ALTR_WRAP_NONVOID(void *, profile, (sqlite3 *db, void (*xProfile)(void*,const char*,sqlite3_uint64), void *pArg), (db, xProfile, pArg))
ALTR_WRAP_VOID(progress_handler, (sqlite3 *db, int nOps, int (*xProgress)(void*), void *pArg), (db, nOps, xProgress, pArg))

// --- C++ Namespace (SQLITE3_ALTERA_WRAPPER) ---
#ifdef __cplusplus
extern "C" {
#endif

int _ZN22SQLITE3_ALTERA_WRAPPER22sqlite3_open_read_onlyEPKcPP7sqlite3(const char *zFilename, sqlite3 **ppDb) {
    return sqlite3_open_v2(zFilename, ppDb, SQLITE_OPEN_READONLY, NULL);
}

int _ZN22SQLITE3_ALTERA_WRAPPER23sqlite3_close_even_busyEP7sqlite3(sqlite3 *db) {
    return sqlite3_close_v2(db);
}

#ifdef __cplusplus
}
#endif
