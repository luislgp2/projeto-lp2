#include "common.h"
#include "libtslog.h"
#include <process.h>

SOCKET sock;
int connected = 0;

unsigned __stdcall receive_handler(void* arg) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    
    while (connected) {
        bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("\nConexão com o servidor perdida.\n");
            connected = 0;
            break;
        }
        
        buffer[bytes_received] = '\0';
        printf("\n%s\n", buffer);
        printf("Você: ");
        fflush(stdout);
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    char username[50];
    char message[BUFFER_SIZE];
    char server_ip[16] = "127.0.0.1";
    int port = SERVER_PORT;
    
    // Configuração de IP e porta via argumentos
    if (argc >= 2) strncpy(server_ip, argv[1], 15);
    if (argc >= 3) port = atoi(argv[2]);
    
    printf("=== Cliente de Chat ===\n");
    printf("Conectando ao servidor %s:%d\n\n", server_ip, port);
    
    printf("Digite seu username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';
    
    // Inicializar Winsock
    setup_winsock();
    
    // Criar socket e conectar
    sock = create_socket();
    connect_to_server(sock, server_ip, port);
    
    // Enviar username
    send(sock, username, strlen(username), 0);
    
    printf("Conectado! Digite '/quit' para sair.\n\n");
    
    // Iniciar thread de recebimento
    connected = 1;
    HANDLE recv_thread = (HANDLE)_beginthreadex(NULL, 0, receive_handler, NULL, 0, NULL);
    
    // Loop de envio de mensagens
    while (connected) {
        printf("Você: ");
        fflush(stdout);
        
        if (!fgets(message, sizeof(message), stdin)) break;
        
        message[strcspn(message, "\n")] = '\0';
        
        if (strlen(message) == 0) continue;
        
        if (strcmp(message, "/quit") == 0) {
            connected = 0;
            break;
        }
        
        if (send(sock, message, strlen(message), 0) <= 0) {
            printf("Erro ao enviar mensagem.\n");
            connected = 0;
            break;
        }
    }
    
    // Limpeza
    if (recv_thread != NULL) {
        WaitForSingleObject(recv_thread, 1000);
        CloseHandle(recv_thread);
    }
    
    closesocket(sock);
    WSACleanup();
    printf("Conexão encerrada.\n");
    
    return 0;
}

void setup_winsock() {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        printf("Falha ao inicializar Winsock\n");
        exit(1);
    }
}

SOCKET create_socket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Falha ao criar socket\n");
        WSACleanup();
        exit(1);
    }
    return sock;
}

void connect_to_server(SOCKET sock, const char* server_ip, int port) {
    struct sockaddr_in server_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Falha ao conectar com o servidor\n");
        closesocket(sock);
        WSACleanup();
        exit(1);
    }
}
