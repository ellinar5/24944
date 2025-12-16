# Makefile (Solaris make)

CC = cc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS =

SERVER_SRC = server-select.c
CLIENT_SRC = client_auto.c

SERVER = server
CLIENT = client

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(LDFLAGS)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f $(SERVER) $(CLIENT) *.o
