//network.h
#ifndef NETWORK_H
#define NETWORK_H

int create_socket(const char *registry_ip, int registry_port);
int send_data(int sock, const void *data, size_t size);
int receive_data(int sock, void *buffer, size_t size);
void close_socket(int sock);

#endif
