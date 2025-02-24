// network.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "network.h"

int create_socket(const char *registry_ip, int registry_port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in registry_addr;
    memset(&registry_addr, 0, sizeof(registry_addr));
    registry_addr.sin_family = AF_INET;
    registry_addr.sin_port = htons(registry_port);

    if (inet_pton(AF_INET, registry_ip, &registry_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address\n");
        close(sock);
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&registry_addr, sizeof(registry_addr)) == -1) {
        perror("connect");
        close(sock);
        return -1;
    }

    return sock;
}

int send_data(int sock, const void *data, size_t size) {
    ssize_t sent_bytes = send(sock, data, size, 0);
    if (sent_bytes == -1) {
        perror("send");
        return -1;
    }
    return sent_bytes;
}

int receive_data(int sock, void *buffer, size_t size) {
    ssize_t received_bytes = recv(sock, buffer, size, 0);
    if (received_bytes == -1) {
        perror("recv");
        return -1;
    }
    return received_bytes;
}

void close_socket(int sock) {
    close(sock);
}
