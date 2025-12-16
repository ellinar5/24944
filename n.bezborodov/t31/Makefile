# Makefile (Solaris make)

CC = cc
CFLAGS = -O2
LDFLAGS =

SERVER_SRC = server-select.c
CLIENT_SRC = client_auto.c

SERVER = server
CLIENT = client

# Если используем gcc — добавим предупреждения
GCC = $(findstring gcc,$(CC))
CFLAGS_GCC = -Wall -Wextra

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(GCC:$(GCC)=$(CFLAGS_GCC)) -o $@ $(SERVER_SRC) $(LDFLAGS)

$(CLIENT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(GCC:$(GCC)=$(CFLAGS_GCC)) -o $@ $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f $(SERVER) $(CLIENT) *.o
