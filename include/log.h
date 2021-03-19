#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_LEN 1024

// if path is NULL, log to stdout
void log_msg(char *path, char *msg);

#endif // "log.h" included
