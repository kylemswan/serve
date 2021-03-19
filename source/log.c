#include "log.h"

void log_msg(char *path, char *msg) {
    time_t t = time(NULL);
    char *stamp = ctime(&t);

    // strip the new line character at the end of the time stamp
    stamp[strlen(stamp) - 1] = '\0';

    if (!path) {
        fprintf(stdout, "* [%s] --- ", stamp);
        fprintf(stdout, "%s\n", msg);
    } else {
        FILE *fp = fopen(path, "a");
        fprintf(fp, "* [%s] --- ", stamp);
        fprintf(fp, "%s\n", msg);
        fclose(fp);
    }
}
