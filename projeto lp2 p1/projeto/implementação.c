#include "libtslog.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdarg.h>

CRITICAL_SECTION log_cs;

void log_init() {
    InitializeCriticalSection(&log_cs);
}

void log_message(int level, const char* format, ...) {
    EnterCriticalSection(&log_cs);
    
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    const char* level_str;
    switch(level) {
        case LOG_LEVEL_INFO: level_str = "INFO"; break;
        case LOG_LEVEL_WARNING: level_str = "WARNING"; break;
        case LOG_LEVEL_ERROR: level_str = "ERROR"; break;
        default: level_str = "UNKNOWN"; break;
    }
    
    printf("[%s] [%s] ", timestamp, level_str);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
    
    LeaveCriticalSection(&log_cs);
}

void log_cleanup() {
    DeleteCriticalSection(&log_cs);
}
