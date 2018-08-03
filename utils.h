#ifndef __UTILS_H__
#define __UTILS_H__

#define LOG_ERROR(fmt, args...) \
    do {\
        fprintf(stderr, "[%s:%d][%s] ", __FILE__, __LINE__,  __FUNCTION__); \
        fprintf(stderr, fmt, ##args); \
        fprintf(stderr, "\n"); \
    } while (0)


#endif// __UTILS_H__
