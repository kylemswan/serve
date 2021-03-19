#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>

struct q_node {
    int fd;
    struct q_node *next;
};

void enqueue(int fd);
int dequeue();

#endif // "queue.h" included
