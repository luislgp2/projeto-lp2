#ifndef COMMON_H
#define COMMON_H

#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

typedef struct {
    SOCKET socket;
    char username[50];
    int active;
} Client;

typedef struct {
    Client* clients[MAX_CLIENTS];
    int count;
    CRITICAL_SECTION cs;
} ClientList;

// Funções do servidor
void client_list_init(ClientList* list);
void client_list_add(ClientList* list, Client* client);
void client_list_remove(ClientList* list, SOCKET socket);
void broadcast_message(ClientList* list, const char* message, SOCKET sender);
Client* find_client_by_socket(ClientList* list, SOCKET socket);

// Funções do cliente
void setup_winsock();
SOCKET create_socket();
void connect_to_server(SOCKET sock, const char* server_ip, int port);

#endif
