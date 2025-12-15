CC=gcc
CFLAGS=-Wall -Wextra -O2

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

demo: all
	./server & \
	sleep 1; \
	echo "client1: hello" | ./client; \
	echo "client2: bye" | ./client; \
	kill $$! 2>/dev/null || true

clean:
	rm -f server client
	rm -f /tmp/uds_upper.sock
