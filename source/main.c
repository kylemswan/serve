#include "log.h"

// system networking headers
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// maximum length for string buffers
#define BUFF_SIZE 4096
#define MAX_LINE 1024
#define MAX_ATTR 64

// standard HTTP response code headers
#define HDR_200 "HTTP/1.0 200 OK\r\n\r\n"
#define HDR_404 "HTTP/1.0 404 NOT FOUND\r\n\r\n"

// global variables so that they can be reached by the SIGINT teardown handler
int server_fd;
bool logging = false;
char *log_path = NULL;

// callback function for the signal handler - has to take an integer parameter
void teardown(int dummy) {
    close(server_fd);
    if (logging) {
        log_msg(log_path, "Interrupt received, closing server socket");
    }
    exit(EXIT_SUCCESS);
}

// check each call for -1 return failures
void check(int return_value, char *description) {
    if (return_value == -1) {
        perror(description);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <PORT> [--log [<stream>]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);

    // configure logging if requested
    char msg[MAX_LINE];
    if (argc > 2 && (strncmp(argv[2], "--log", MAX_ATTR) == 0)) {
        logging = true;
        if (argc == 4) {
            log_path = argv[3];
        }
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(server_fd, "At socket initialisation");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    check(bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)), "At bind");
    check(listen(server_fd, 1), "At bind");

    if (logging) {
        snprintf(msg, MAX_LINE, "Server active at http://localhost:%d", port);
        log_msg(log_path, msg);
    }

    // keep accepting new connections until interrupted
    signal(SIGINT, &teardown);
    while (1) {
        // store the client address information
        struct sockaddr_in c_addr;
        socklen_t c_len;

        int client_fd = accept(server_fd, (struct sockaddr *) &c_addr, &c_len);
    
        // convert client address struct into printable IP format
        char c_ip[c_len];
        inet_ntop(AF_INET, &c_addr, c_ip, c_len);

        // parse the client request
        char req_header[MAX_LINE];
        char method[MAX_ATTR];
        char resource[MAX_ATTR];
        char protocol[MAX_ATTR];
        read(client_fd, req_header, MAX_LINE);
        sscanf(req_header, "%s %s %s\r\n", method, resource, protocol);

        // prepend the public directory to confine paths
        char path[MAX_LINE];
        if (strncmp(resource, "/", MAX_LINE) == 0) {
            snprintf(path, MAX_LINE, "public/index.html");
        } else {
            snprintf(path, MAX_LINE, "public/%s", resource);
        }

        // check that the file exists - set header accordingly
        char res_header[MAX_LINE];
        if (access(path, F_OK) == -1) {
            snprintf(res_header, MAX_LINE, HDR_404);
            snprintf(path, MAX_LINE, "public/404.html");
        } else {
            snprintf(res_header, MAX_LINE, HDR_200);
        }

        // write the header first
        write(client_fd, res_header, strlen(res_header));

        // write the requested file in buffered chunks
        FILE *fp = fopen(path, "r");
        size_t bytes;
        size_t total = 0;
        char buff[BUFF_SIZE];
        while ((bytes = fread(buff, 1, BUFF_SIZE, fp)) > 0) {
            write(client_fd, buff, bytes);
            total += bytes;
        }
        fclose(fp);

        // log transaction details
        if (logging) {
            snprintf(msg, MAX_LINE, "IP: %s, resource: %s, bytes: %ld",
                c_ip, resource, total);
            log_msg(log_path, msg);
        }

        close(client_fd);
    }

    return EXIT_SUCCESS;
}
