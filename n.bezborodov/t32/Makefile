CC=gcc
CFLAGS=-Wall -Wextra -O2
LDLIBS=-lsocket -lnsl -lrt

all: server client

server: server_async.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

client: client_auto.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f server client /tmp/uppercase_socket_nbezborodov *.o
