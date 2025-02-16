#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT "80"
#define MAX_CHUNK_SIZE 1000
#define MAX_TAG_OVERLAP 20 // increased to handle more complex tag overlaps

int lookup_and_connect(const char *host, const char *service);

int main(int argc, char *argv[]) {
    char *host = "www.ecst.csuchico.edu";
    int chunk_size;
    int s; // socket descriptor
    char buf[MAX_CHUNK_SIZE + MAX_TAG_OVERLAP]; // extra space for overlap between chunks
    int bytes_received;
    int total_bytes = 0; // total bytes received
    int h1_count = 0; // number of <h1> tags
    const char *h1_tag = "<h1>";
    const char *request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";

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

    // Send HTTP GET request in chunks if necessary
    size_t total_sent = 0;
    size_t request_len = strlen(request);
    while (total_sent < request_len) {
        ssize_t bytes_sent = send(s, request + total_sent, request_len - total_sent, 0);
        if (bytes_sent == -1) {
            perror("send");
            close(s);
            exit(1);
        }
        total_sent += bytes_sent;
    }

    // Variable to hold the leftover data (potential part of an <h1> tag across chunks)
    int overlap_len = 0;
    
    // Receive and process data in chunks
    while ((bytes_received = recv(s, buf + overlap_len, chunk_size, 0)) > 0) {
        total_bytes += bytes_received;
        
        // Null-terminate the received data
        buf[overlap_len + bytes_received] = '\0';

        // Count <h1> tags in the current buffer
        char *ptr = buf;
        while ((ptr = strstr(ptr, h1_tag)) != NULL) {
            h1_count++;
            ptr += strlen(h1_tag); // Move the pointer forward to avoid counting the same tag again
        }

        // Handle the leftover data for next chunk (checking if a tag spans across chunks)
        if (bytes_received == chunk_size) {
            // Check if the last part of the current chunk has an incomplete <h1> tag
            int end_overlap_len = strlen(h1_tag) - 1;
            if (end_overlap_len > 0) {
                // Copy part of the current chunk to the beginning of the next one
                memcpy(buf, buf + chunk_size - end_overlap_len, end_overlap_len);
                overlap_len = end_overlap_len;
            } else {
                overlap_len = 0;
            }
        }
    }

    if (bytes_received == -1) {
        perror("recv");
        close(s);
        exit(1);
    }

    // Check if connection was closed by the server
    if (bytes_received == 0) {
        printf("Connection closed by the server\n");
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

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

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

