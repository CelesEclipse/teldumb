#ifndef _LOGGER_H_
#define _LOGGER_H_

#define LOG_TIME_SIZE   36
typedef enum {
    GW_LOG_DEBUG,
    GW_LOG_INFO,
    GW_LOG_WARN,
    GW_LOG_ERROR
} GWLogLevel;

int logger_init(const char * file, GWLogLevel runtime_level);
void logger_log(GWLogLevel level, const char * file, int line, const char * fmt, ...);
void logger_shutdown(void);

#define LOG_INF(fmt, ...) logger_log(GW_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WRN(fmt, ...) logger_log(GW_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) logger_log(GW_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__);

#ifdef DEBUG_BUILD
    #define LOG_DBG(fmt, ...) logger_log(GW_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#else
    #define LOG_DBG(fmt, ...) do {} while (0)
#endif

#endif
