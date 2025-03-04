// protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>
#include <arpa/inet.h>

#define JOIN_ACTION 0
#define PUBLISH_ACTION 1
#define SEARCH_ACTION 2

void create_join_request(uint8_t *buffer, uint32_t peer_id);
void create_publish_request(uint8_t *buffer, int file_count, const char *filenames);
void create_search_request(uint8_t *buffer, const char *filename);
int parse_search_response(const uint8_t *buffer, uint32_t *peer_id, char *peer_ip, uint16_t *peer_port);

#endif
