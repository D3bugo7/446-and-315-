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
    // initialization of variables used in the program
    char *host = "www.ecst.csuchico.edu"; // assign the server host to a host variable
    int chunk_size;
    int s; // socked descriptor
    char buf[MAX_CHUNK_SIZE]; // buffer to store received data
    int bytes_received;
    int total_bytes = 0; // total bytes received
    int h1_count = 0; // number of h1 tags
    const char *h1_tag = "<h1>"; // h1 tag string to search
    const char *request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n"; // http get request

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

    // Lookup IP and connect to the server
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
            ptr += strlen(h1_tag); // move the pointer forward to avoid counting the same tag again
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

    close(s); // close the socket
    return 0;
}

int lookup_and_connect(const char *host, const char *service) {
    struct addrinfo hints;
    struct addrinfo *rp, *result;
    int s;

    // Translate host name into peer's IP address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC instead of AF_INET to allow ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; // use TCP
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    // get address information
    if ((s = getaddrinfo(host, service, &hints, &result)) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    // Iterate through the address list and try to connect
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue; // find the next address if socket creation fails
        }

        if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
            break; // successful connection
        }

        close(s); // close socket if connection fails
    }

    if (rp == NULL) { // no address 
        perror("connect");
        freeaddrinfo(result); // free addrinfo
        return -1;
    }

    freeaddrinfo(result); // free addrinfo
    return s; // return the connected socket discriptor
}
