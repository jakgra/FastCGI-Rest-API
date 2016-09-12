#ifndef jak_dbg_h__
#define jak_dbg_h__

/**
 * @file
 * @brief Debugging macros
 *
 * This file contains useful macros for debugging purposes and for error handling
 * using the jump technique:
 * check if a condition is true else jump to the cleanup part of the function
 * (normally somewhere at the bottom)
 */


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"




#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef OS_IS_ANDROID
#include <android/log.h>
#define jak_print(...) __android_log_print(ANDROID_LOG_DEBUG, "moj-debug", ##__VA_ARGS__)
#else
#define jak_print(...) fprintf( stderr, ##__VA_ARGS__ )
#endif

#ifdef NDEBUG
#define debug(M)
#define debug_v(M, ...)
#else
#define debug(M) jak_print( "DEBUG %s:%d: " M "\n", __FILE__, __LINE__)
#define debug_v(M, ...) jak_print( "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno==0 ? "None" : strerror(errno))

#define log_err(M) jak_print( "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno())
#define log_err_v(M, ...) jak_print( "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) jak_print( "[WARN] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define log_info(M, ...) jak_print( "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define sentinel(M, ...) { log_err(M, ##__VA_ARGS__); errno=0; goto error; }

#define log_err_empty() jak_print( "[ERROR] (%s:%d: errno: %s)\n", __FILE__, __LINE__, clean_errno())

#define check(A, B) if(!(A)) { log_err_empty(); errno=0; goto B; }




#pragma clang diagnostic pop
#pragma GCC diagnostic pop


#endif
