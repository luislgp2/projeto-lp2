CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_WIN32_WINNT=0x0600
LIBS = -lws2_32
SRC_DIR = src
INCLUDE_DIR = include
BIN_DIR = bin

SOURCES_SERVER = $(SRC_DIR)/server.c $(SRC_DIR)/libtslog.c
SOURCES_CLIENT = $(SRC_DIR)/client.c $(SRC_DIR)/libtslog.c

all: server client

server: $(SOURCES_SERVER)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(SOURCES_SERVER) -o $(BIN_DIR)/server.exe $(LIBS)

client: $(SOURCES_CLIENT)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $(SOURCES_CLIENT) -o $(BIN_DIR)/client.exe $(LIBS)

clean:
	del /Q $(BIN_DIR)\*.exe

.PHONY: all clean
