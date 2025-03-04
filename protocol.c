// protocol.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol.h"

void create_join_request(uint8_t *buffer, uint32_t peer_id) {
    buffer[0] = JOIN_ACTION;
    uint32_t network_peer_id = htonl(peer_id);
    memcpy(buffer + 1, &network_peer_id, sizeof(network_peer_id));
}

void create_publish_request(uint8_t *buffer, int file_count, const char *filenames) {
    buffer[0] = PUBLISH_ACTION;
    uint32_t network_count = htonl(file_count);
    memcpy(buffer + 1, &network_count, sizeof(network_count));
    strcpy((char *)buffer + 5, filenames);
}

void create_search_request(uint8_t *buffer, const char *filename) {
    buffer[0] = SEARCH_ACTION;
    strcpy((char *)buffer + 1, filename);
}

int parse_search_response(const uint8_t *buffer, uint32_t *peer_id, char *peer_ip, uint16_t *peer_port) {
    *peer_id = ntohl(*(uint32_t *)buffer);
    struct in_addr addr;
    memcpy(&addr.s_addr, buffer + 4, sizeof(addr.s_addr));
    inet_ntop(AF_INET, &addr, peer_ip, INET_ADDRSTRLEN);
    *peer_port = ntohs(*(uint16_t *)(buffer + 8));
    return 0;
}
