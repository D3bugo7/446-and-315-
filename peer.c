// peer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "network.h"
#include "protocol.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <registry_ip> <registry_port> <peer_id>\n", argv[0]);
        return 1;
    }

    const char *registry_ip = argv[1];
    int registry_port = atoi(argv[2]);
    uint32_t peer_id = atoi(argv[3]);

    int sock = create_socket(registry_ip, registry_port);
    if (sock == -1) {
        return 1;
    }

    char command[256];

    while (1) {
        printf("Enter a command: ");
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "JOIN") == 0) {
            uint8_t join_request[5];
            create_join_request(join_request, peer_id);
            send_data(sock, join_request, sizeof(join_request));
        } else if (strcmp(command, "SEARCH") == 0) {
            // ... (Implement SEARCH command logic)
        } else if (strcmp(command, "PUBLISH") == 0) {
            // ... (Implement PUBLISH command logic)
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    close_socket(sock);
    return 0;
}
