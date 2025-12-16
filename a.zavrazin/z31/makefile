# Makefile for UNIX-domain socket project

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -std=c99

# Targets
TARGETS = server client launcher

# Default target
all: $(TARGETS)

# Server
server: server.c
	$(CC) $(CFLAGS) -o server server.c

# Client
client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Launcher
launcher: launcher.c
	$(CC) $(CFLAGS) -o launcher launcher.c

# Clean rule
clean:
	rm -f $(TARGETS) socket

# Phony targets
.PHONY: all clean
