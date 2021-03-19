#include "log.h"

pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_msg(char *path, char *msg) {
    time_t t = time(NULL);
    char *stamp = ctime(&t);

    // strip the new line character at the end of the time stamp
    stamp[strlen(stamp) - 1] = '\0';

    if (!path) {
        fprintf(stdout, "* [%s] --- ", stamp);
        fprintf(stdout, "%s\n", msg);
    } else {
        // critical region, need to lock here to avoid clobbering text!
        pthread_mutex_lock(&io_mutex);

        FILE *fp = fopen(path, "a");
        fprintf(fp, "* [%s] --- ", stamp);
        fprintf(fp, "%s\n", msg);
        fclose(fp);

        pthread_mutex_unlock(&io_mutex);
    }
}
