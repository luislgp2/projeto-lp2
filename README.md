# projeto-lp2# Chat Server

Este projeto é um sistema de chat multiusuário desenvolvido em C para Windows, utilizando sockets e threads.

## Estrutura do Projeto

```
chat_server/
├── src/
│   ├── server.c         # Código-fonte do servidor
│   ├── client.c         # Código-fonte do cliente
│   ├── libtslog.c       # Logger thread-safe
│   └── libtslog.h       # Header do logger
├── include/
│   └── common.h         # Definições e funções comuns
├── Makefile             # Script de compilação
├── README.md            # Este arquivo
└── scripts/
    └── test_clients.bat # Script para testar múltiplos clientes
```

## Como Compilar

1. Instale o GCC (MinGW) no Windows.
2. No terminal, navegue até a pasta do projeto e execute:
   ```
   mingw32-make
   ```
   Os executáveis serão gerados na pasta `src/`.

## Como Executar

### Servidor

No terminal:
```
src\server.exe
```

### Cliente

Em outro terminal:
```
src\client.exe 127.0.0.1 8080
```
(Substitua o IP e a porta conforme necessário.)

Você pode rodar múltiplos clientes para testar o chat.

## Teste Automático

Execute o script para abrir múltiplos clientes:
```
scripts\test_clients.bat
```

## Funcionalidades

- Suporte a múltiplos clientes simultâneos.
- Mensagens de broadcast para todos os clientes conectados.
- Comando `/quit` para sair do chat.
- Logging de eventos do servidor.

---

Desenvolvido
