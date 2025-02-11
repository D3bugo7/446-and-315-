#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT "80"
#define MAX_CHUNK_SIZE 1000

/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int main(int argc, char *argv[]) {
    char *host = "www.ecst.csuchico.edu";
    int chunk_size;
    int s;
    char buf[MAX_CHUNK_SIZE];
    int bytes_received;
    int total_bytes = 0;
    int h1_count = 0;
    const char *h1_tag = "<h1>";
    const char *request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";

    // Check command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <chunk_size>\n", argv[0]);
        exit(1);
    }
    chunk_size = atoi(argv[1]);
    if (chunk_size <= 0 || chunk_size > MAX_CHUNK_SIZE) {
        fprintf(stderr, "Chunk size must be a positive integer <= %d\n", MAX_CHUNK_SIZE);
        exit(1);
    }

    // Lookup IP and connect to server
    if ((s = lookup_and_connect(host, SERVER_PORT)) < 0) {
        exit(1);
    }

    // Send HTTP GET request
    if (send(s, request, strlen(request), 0) == -1) {
        perror("send");
        close(s);
        exit(1);
    }

    // Receive and process data
    while ((bytes_received = recv(s, buf, chunk_size, 0)) > 0) {
        total_bytes += bytes_received;

        // Count <h1> tags
        char *ptr = buf;
        while ((ptr = strstr(ptr, h1_tag)) != NULL) {
            h1_count++;
            ptr += strlen(h1_tag);
        }
    }

    // Check for errors in recv
    if (bytes_received == -1) {
        perror("recv");
        close(s);
        exit(1);
    }

    // Print results
    printf("Number of <h1> tags: %d\n", h1_count);
    printf("Number of bytes: %d\n", total_bytes);

    close(s);
    return 0;
}

int lookup_and_connect(const char *host, const char *service) {
    struct addrinfo hints;
    struct addrinfo *rp, *result;
    int s;

    // Translate host name into peer's IP address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC instead of AF_INET
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if ((s = getaddrinfo(host, service, &hints, &result)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    // Iterate through the address list and try to connect
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }

        if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }

        close(s);
    }

    if (rp == NULL) {
        perror("connect");
        freeaddrinfo(result);
        return -1;
    }

    freeaddrinfo(result);
    return s;
}
