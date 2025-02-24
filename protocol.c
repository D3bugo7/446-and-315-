// protocol.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "protocol.h"

// Example: Create a JOIN request
void create_join_request(uint8_t *buffer, uint32_t peer_id) {
    buffer[0] = JOIN_ACTION; // JOIN_ACTION defined as 0 in protocol.h
    uint32_t network_peer_id = htonl(peer_id);
    memcpy(buffer + 1, &network_peer_id, sizeof(network_peer_id));
}

// Example: Parse a SEARCH response
int parse_search_response(const uint8_t *buffer, uint32_t *peer_id, char *peer_ip, uint16_t *peer_port) {
    *peer_id = ntohl(*(uint32_t *)buffer);
    struct in_addr addr;
    memcpy(&addr.s_addr, buffer + 4, sizeof(addr.s_addr));
    inet_ntop(AF_INET, &addr, peer_ip, INET_ADDRSTRLEN);
    *peer_port = ntohs(*(uint16_t *)(buffer + 8));
    return 0;
}

// more protocol functions will be added here.
