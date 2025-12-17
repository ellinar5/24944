CC=gcc
CFLAGS=-Wall -Wextra -O2
LDLIBS=-lsocket -lnsl -lrt

all: server client

server: server_async.c
	$(CC) $(CFLAGS) server_async.c -o server $(LDLIBS)

client: client_auto.c
	$(CC) $(CFLAGS) client_auto.c -o client $(LDLIBS)

clean:
	rm -f server client /tmp/uppercase_socket_nbezborodov *.o
