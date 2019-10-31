#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef LOGGER_H
#define LOGGER_H

char current_log_filename[200];

// FILE* log_file =

void log_to_file (char *str) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char filename[200];
    sprintf(filename, "logs/%d%d%d", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday);
    strcat(filename, ".log");
    FILE* log_file = fopen(filename, "a");
    if (log_file == NULL) {
        FILE* log_file = fopen(filename, "w");
    }
    fprintf(log_file, "[%d-%d-%d %d:%d:%d] %s\n", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, str);

    fclose(log_file);
}

char* get_current_logfile() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(current_log_filename, "logs/%d%d%d", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday);
    strcat(current_log_filename, ".log");

    return current_log_filename;
}

#endif
