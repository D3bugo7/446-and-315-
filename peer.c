// peer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
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
            printf("Enter a file name: ");
            char filename[101];
            if (fgets(filename, sizeof(filename), stdin) == NULL) break;
            filename[strcspn(filename, "\n")] = 0;

            uint8_t search_request[105]; // Max filename 100 + action byte + null terminator
            create_search_request(search_request, filename);
            send_data(sock, search_request, 1 + strlen(filename) + 1);

            uint8_t search_response[10];
            if (receive_data(sock, search_response, sizeof(search_response)) != sizeof(search_response)) {
                fprintf(stderr, "Error receiving search response.\n");
                continue;
            }

            uint32_t found_peer_id;
            char found_peer_ip[INET_ADDRSTRLEN];
            uint16_t found_peer_port;

            if (parse_search_response(search_response, &found_peer_id, found_peer_ip, &found_peer_port) == 0) {
                if (found_peer_id == 0 && found_peer_port == 0) {
                    printf("File not indexed by registry\n");
                } else {
                    printf("File found at\nPeer %u\n%s:%u\n", found_peer_id, found_peer_ip, found_peer_port);
                }
            } else {
                fprintf(stderr, "Error parsing search response.\n");
            }
        } else if (strcmp(command, "PUBLISH") == 0) {
            DIR *d;
            struct dirent *dir;
            d = opendir("SharedFiles");
            if (d) {
                char filenames[1200];
                int file_count = 0;
                int offset = 0;

                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_type == DT_REG) {
                        strcpy(filenames + offset, dir->d_name);
                        offset += strlen(dir->d_name) + 1;
                        file_count++;
                    }
                }
                closedir(d);

                uint8_t publish_request[1205]; //Action byte + int count + filenames
                create_publish_request(publish_request, file_count, filenames);
                send_data(sock, publish_request, 5 + offset);
            } else {
                perror("SharedFiles");
            }
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    close_socket(sock);
    return 0;
}