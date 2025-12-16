# Makefile (Solaris)

CC      ?= cc
CFLAGS  ?= -O2
CFLAGS  += -Wall -Wextra
LDFLAGS ?=

SERVER_SRC = server-select.c
CLIENT_SRC = client_auto.c

SERVER = server
CLIENT = client

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(SERVER) $(CLIENT) *.o
