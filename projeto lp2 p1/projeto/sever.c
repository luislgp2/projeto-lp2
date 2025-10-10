#include "common.h"
#include "libtslog.h"
#include <process.h>

ClientList client_list;

unsigned __stdcall client_handler(void* data) {
    Client* client = (Client*)data;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    // Receber username do cliente
    bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        LOG_ERROR("Falha ao receber username do cliente");
        closesocket(client->socket);
        free(client);
        return 1;
    }
    
    buffer[bytes_received] = '\0';
    strncpy(client->username, buffer, sizeof(client->username) - 1);
    client->active = 1;
    
    LOG_INFO("Cliente conectado: %s (socket: %d)", client->username, client->socket);
    
    // Broadcast de entrada
    char welcome_msg[BUFFER_SIZE];
    snprintf(welcome_msg, sizeof(welcome_msg), "[SERVIDOR] %s entrou no chat", client->username);
    broadcast_message(&client_list, welcome_msg, client->socket);
    
    // Loop principal de mensagens
    while (client->active) {
        bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            LOG_INFO("Cliente desconectado: %s", client->username);
            break;
        }
        
        buffer[bytes_received] = '\0';
        
        // Verificar comando de saída
        if (strcmp(buffer, "/quit") == 0) {
            break;
        }
        
        // Formatar e broadcast mensagem
        char formatted_msg[BUFFER_SIZE];
        snprintf(formatted_msg, sizeof(formatted_msg), "[%s] %s", client->username, buffer);
        
        LOG_INFO("Mensagem de %s: %s", client->username, buffer);
        broadcast_message(&client_list, formatted_msg, client->socket);
    }
    
    // Remover cliente e broadcast saída
    char leave_msg[BUFFER_SIZE];
    snprintf(leave_msg, sizeof(leave_msg), "[SERVIDOR] %s saiu do chat", client->username);
    broadcast_message(&client_list, leave_msg, client->socket);
    
    client_list_remove(&client_list, client->socket);
    closesocket(client->socket);
    free(client);
    
    return 0;
}

int main() {
    WSADATA wsa_data;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_len = sizeof(client_addr);
    
    log_init();
    LOG_INFO("Iniciando servidor de chat...");
    
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        LOG_ERROR("Falha ao inicializar Winsock");
        return 1;
    }
    
    // Criar socket do servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        LOG_ERROR("Falha ao criar socket");
        WSACleanup();
        return 1;
    }
    
    // Configurar endereço do servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        LOG_ERROR("Falha no bind");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    // Listen
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        LOG_ERROR("Falha no listen");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    LOG_INFO("Servidor ouvindo na porta %d", SERVER_PORT);
    
    // Inicializar lista de clientes
    client_list_init(&client_list);
    
    // Loop principal de aceitação de conexões
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == INVALID_SOCKET) {
            LOG_ERROR("Falha ao aceitar conexão");
            continue;
        }
        
        LOG_INFO("Nova conexão aceita de %s:%d", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        // Verificar se há espaço para novo cliente
        EnterCriticalSection(&client_list.cs);
        if (client_list.count >= MAX_CLIENTS) {
            LOG_WARNING("Número máximo de clientes atingido. Conexão recusada.");
            const char* msg = "[SERVIDOR] Servidor cheio. Tente novamente mais tarde.";
            send(client_socket, msg, strlen(msg), 0);
            closesocket(client_socket);
            LeaveCriticalSection(&client_list.cs);
            continue;
        }
        LeaveCriticalSection(&client_list.cs);
        
        // Criar novo cliente
        Client* new_client = (Client*)malloc(sizeof(Client));
        new_client->socket = client_socket;
        new_client->active = 0;
        
        // Adicionar à lista
        client_list_add(&client_list, new_client);
        
        // Criar thread para o cliente
        HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, client_handler, new_client, 0, NULL);
        if (thread == NULL) {
            LOG_ERROR("Falha ao criar thread para cliente");
            client_list_remove(&client_list, client_socket);
            closesocket(client_socket);
            free(new_client);
        } else {
            CloseHandle(thread);
        }
    }
    
    closesocket(server_socket);
    WSACleanup();
    log_cleanup();
    return 0;
}

// Implementações das funções da lista de clientes
void client_list_init(ClientList* list) {
    InitializeCriticalSection(&list->cs);
    list->count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        list->clients[i] = NULL;
    }
}

void client_list_add(ClientList* list, Client* client) {
    EnterCriticalSection(&list->cs);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (list->clients[i] == NULL) {
            list->clients[i] = client;
            list->count++;
            break;
        }
    }
    LeaveCriticalSection(&list->cs);
}

void client_list_remove(ClientList* list, SOCKET socket) {
    EnterCriticalSection(&list->cs);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (list->clients[i] != NULL && list->clients[i]->socket == socket) {
            list->clients[i] = NULL;
            list->count--;
            break;
        }
    }
    LeaveCriticalSection(&list->cs);
}

void broadcast_message(ClientList* list, const char* message, SOCKET sender) {
    EnterCriticalSection(&list->cs);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (list->clients[i] != NULL && list->clients[i]->active && 
            list->clients[i]->socket != sender) {
            send(list->clients[i]->socket, message, strlen(message), 0);
        }
    }
    LeaveCriticalSection(&list->cs);
}

Client* find_client_by_socket(ClientList* list, SOCKET socket) {
    EnterCriticalSection(&list->cs);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (list->clients[i] != NULL && list->clients[i]->socket == socket) {
            Client* client = list->clients[i];
            LeaveCriticalSection(&list->cs);
            return client;
        }
    }
    LeaveCriticalSection(&list->cs);
    return NULL;
}
