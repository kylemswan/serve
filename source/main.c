#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_LINE 1024
#define MAX_ATTR 64

#define HDR_200 "HTTP/1.0 200 OK\r\n\r\n"
#define HDR_404 "HTTP/1.0 404 NOT FOUND\r\n\r\n"

// server file descriptor - global so that the SIGINT handler can reach it
int server_fd;

void teardown(int dummy) {
    close(server_fd);
    printf("\nInterrupt received - closing server socket...\n");
    exit(EXIT_SUCCESS);
}

// check each call for -1 return failures
void check(int return_value, char *description) {
    if (return_value == -1) {
        perror(description);
        exit(EXIT_FAILURE);
    }
}

// return the size of a requested file
int fsize(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return len;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    check(server_fd, "At socket initialisation");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    check(bind(server_fd, (struct sockaddr *) &addr, sizeof(addr)), "At bind");
    check(listen(server_fd, 1), "At bind");

    // keep accepting new connections until interrupted
    signal(SIGINT, &teardown);
    printf("Server active at http://localhost:%d\n", port);
    
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
        sscanf(req_header, "%s %s %s\r\n\r\n", method, resource, protocol);

        // log client information
        printf("IP: %s, Resource: %s\n", c_ip, resource);

        // prepend the public directory to confine paths
        char path[2 * MAX_ATTR];
        if (strcmp(resource, "/") == 0) {
            sprintf(path, "public/index.html");
        } else {
            sprintf(path, "public/%s", resource);
        }

        // check that the file exists - set header accordingly
        char res_header[MAX_ATTR];
        if (access(path, F_OK) == -1) {
            sprintf(res_header, HDR_404);
            sprintf(path, "public/404.html");
        } else {
            sprintf(res_header, HDR_200);
        }

        FILE *fp = fopen(path, "r");
        int size = fsize(fp);
        char *buffer = malloc(size);
        fread(buffer, 1, size, fp);

        // write the header first, then the content
        write(client_fd, res_header, strlen(res_header));
        write(client_fd, buffer, size);

        // close up buffers and file streams!
        fclose(fp);
        free(buffer);
        close(client_fd);
    }

    return EXIT_SUCCESS;
}
