#ifndef SMART_LOGGER_H__
#define SMART_LOGGER_H__

#ifdef __linux__
    #include <string.h>
#endif

//#include <strings.h>
#include <stdio.h>

/*
 * print hex string
 */
inline void hex_print(unsigned char* c, int len, char* pipe) {
    for (int i = 0; i < len; i++) {
        if (i % 0x10 == 0) {
            fprintf(stderr, "\n%s %04x ", pipe, i);
        }
        if (i % 8 == 0) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "%02x ", c[i]);
    }
    fprintf(stderr, "\n\n");
}

/**
 * POSIX does specify that fprintf() calls from different threads of the same process do not interfere 
 * with each other, and if that if they both specify the same target file, their output will not be 
 * intermingled. 
 * POSIX-conforming fprintf() is thread-safe.
 */

/**
 * define logger macro
 */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define DEBUG_LEVEL 1

#if defined DETAIL_MESSAGE 
    #define _info(fmt, args...) \
    do {\
        fprintf(stderr, "[info][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, ##args);\
    } while(0)

    #define _debug(fmt, args...) \
    do {\
        if (DEBUG_LEVEL) fprintf(stderr, "[debug][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, ##args);\
    } while(0)

    #define _error(fmt, args...) \
    do {\
        fprintf(stderr, "[error][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, ##args);\
    } while(0)

#elif defined SIMPLE_MESSAGE 
    #define _info(fmt, args...) \
    do {\
        fprintf(stderr, "[info] " fmt "\n", ##args);\
    } while(0)

    #define _debug(fmt, args...) \
    do {\
        if (DEBUG_LEVEL) fprintf(stderr, "[debug] " fmt "\n", ##args);\
    } while(0)

    #define _error(fmt, args...) \
    do {\
        fprintf(stderr, "[error] " fmt "\n", ##args);\
    } while(0)

#elif defined ZLOG_MESSAGE
    #include <zlog.h>
    #define _info dzlog_info
    #define _debug dzlog_debug
    #define _error dzlog_error

#elif defined WIN32_MESSAGE
    #define _info(fmt, ...) \
    do {\
        fprintf(stderr, "[info][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, __VA_ARGS__);\
    } while(0)

    #define _debug(fmt,...) \
    do {\
        if (DEBUG_LEVEL) fprintf(stderr, "[debug][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, __VA_ARGS__);\
    } while(0)

    #define _error(fmt, ...) \
    do {\
        fprintf(stderr, "[error][%s:%d:%s()] " fmt "\n", __FILENAME__, __LINE__, __func__, __VA_ARGS__);\
    } while(0)
#endif 

#endif//GEN_PROXY_LOGGER_H__