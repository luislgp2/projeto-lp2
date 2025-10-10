#ifndef LIBTSLOG_H
#define LIBTSLOG_H

#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdarg.h>

#define LOG_LEVEL_INFO    0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR   2

extern CRITICAL_SECTION log_cs;

void log_init();
void log_message(int level, const char* format, ...);
void log_cleanup();

#define LOG_INFO(...) log_message(LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_WARNING(...) log_message(LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) log_message(LOG_LEVEL_ERROR, __VA_ARGS__)

#endif
