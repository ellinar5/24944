CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c99

SERVER=server
CLIENT=client

all: $(SERVER) $(CLIENT)

server: server_async.c
	$(CC) $(CFLAGS) -o server server_async.c -lrt -laio

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c

demo: all
	./$(SERVER) & \
	sleep 1; \
	( echo "client1: hello"; sleep 1; echo "client1: bye" ) | ./$(CLIENT) & \
	( echo "client2: abc";   sleep 1; echo "client2: def" ) | ./$(CLIENT) & \
	wait; \
	kill $$! 2>/dev/null || true

clean:
	rm -f $(SERVER) $(CLIENT)
	rm -f /tmp/uds_upper.sock
