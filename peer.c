// peer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "network.h"
#include "protocol.h"

int main(int argc, char *argv[]) {
    // check if there is the correct amount of arguments, if not then return error
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <registry_ip> <registry_port> <peer_id>\n", argv[0]);
        return 1;
    }
    // Initialize P2P variables
    const char *registry_ip = argv[1]; // registry ip address
    int registry_port = atoi(argv[2]); // registery port number
    uint32_t peer_id = atoi(argv[3]); // Peer ID
    // create a socket connection to the registry
    int sock = create_socket(registry_ip, registry_port);
    if (sock == -1) {
        return 1;
    }
    // create a buffer to store user command
    char command[256];
    while (1) {
        printf("Enter a command: ");
        // check if there is an error with input
        if (fgets(command, sizeof(command), stdin) == NULL) break;
        command[strcspn(command, "\n")] = 0; // Remove newline
        // handle the join command
        if (strcmp(command, "JOIN") == 0) {
            uint8_t join_request[5]; // create buffer to store join request
            create_join_request(join_request, peer_id);
            send_data(sock, join_request, sizeof(join_request));
        } else if (strcmp(command, "SEARCH") == 0) { // handle the search command
            char file_name[255]; // create buffer to store file name
            printf("Enter a file name:");
            if (fgets(file_name, sizeof(file_name), stdin) == NULL) break;
            file_name[strcspn(file_name, "\n")] = 0; // remove newline
            // search request
            uint8_t search_request[256];
            create_search_request(search_request,file_name);
            send_data(sock,search_request,strlen(file_name));
            // search response
            uint8_t search_response[10]; // buffer for search response: 4B (peer id) + 4B (peer ipv4 address) + 2B (Peer port number)
            int response_size = recv_data(sock,search_response, sizeof(search_response));
            // process the response
            if (response_size > 0) 
            {
                process_search_response(search_response);
            } else {
                printf("File not indexed by registry");
            }
        } else if (strcmp(command, "PUBLISH") == 0) { // handle the publish command
            // ... (Implement PUBLISH command logic)
        } else if (strcmp(command, "EXIT") == 0) { // end loop if the user wants to exit
            break;
        } else { // handle incorrect commands
            printf("Invalid command.\n");
        }
    }

    close_socket(sock);
    return 0;
}
