#include <stdio.h>
#include <time.h>
#include "logger.h"

static void get_timestamp(char *buffer, size_t max_size) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, max_size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void log_info(const char *content) {
    FILE *f = fopen("app.log", "a");
    if (!f) return;
    char ts[20];
    get_timestamp(ts, sizeof(ts));
    fprintf(f, "[%s] %s\n", ts, content);
    fclose(f);
}

void log_error(const char *content) {
    FILE *f = fopen("app.log", "a");
    if (!f) return;
    char ts[20];
    get_timestamp(ts, sizeof(ts));
    fprintf(f, "[%s] [ERROR] %s\n", ts, content);
    fclose(f);
}