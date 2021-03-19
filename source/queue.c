#include "queue.h"

struct q_node *head = NULL, *tail = NULL;

void enqueue(int fd) {
    // add to the tail
    struct q_node *new = malloc(sizeof(struct q_node));
    new->fd = fd;
    new->next = NULL;
    if (!tail || !head) {
        head = tail = new;
    } else {
        tail->next = new;
        tail = new;
    }        
}

int dequeue() {
    // take from the head
    if (!head) {
        return -1;
    } else {
        int fd = head->fd;
        struct q_node *temp = head;
        head = head->next;
        free(temp);
        return fd;
    }
}
