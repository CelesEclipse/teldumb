#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "gateway/log/logger.h"

static FILE * log_file = NULL;
static GWLogLevel global_level = GW_LOG_INFO;
static const char * lvl_str[] = {"GWDEBUG", "GWINFO", "GWWARNING", "GWERROR"};

int logger_init(const char * file, GWLogLevel runtime_level)
{
    global_level = runtime_level;
    if (file == NULL) {
        log_file = stdout;
        return 0;
    }

    if ((log_file = fopen(file, "a")) == NULL) {
        perror("Failed to open log file");
        return -1;
    }
    return 0;
}

void logger_log(GWLogLevel level, const char * file, int line, const char * fmt, ...)
{
    if (level < global_level || log_file == NULL) return;

    // Get current time
    time_t raw_time = time(NULL);
    struct tm * time_info = localtime(&raw_time);
    char time_str[LOG_TIME_SIZE];
    if (strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info) == 0) {
        // do something
        fprintf(log_file, "[INTERNAL LOG ERROR] failed to format timestamp", file, line);
        snprintf(time_str, sizeof(time_str), "UNKNOWN-TIME");
    }
    fprintf(log_file, " [%s] [%s] [%s:%d] ", time_str, lvl_str[level], file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fprintf(log_file, "\n");
    fflush(log_file);
}

void logger_shutdown(void)
{
    if (log_file != NULL && log_file != stdout) {
        fclose(log_file);
        log_file = NULL;
    }
}
