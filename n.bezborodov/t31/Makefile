CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c99

SERVER=server
CLIENT=client

all: $(SERVER) $(CLIENT)

$(SERVER): server-select.c
	$(CC) $(CFLAGS) -o $(SERVER) server-select.c

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c

run-server: $(SERVER)
	./$(SERVER)

run-client: $(CLIENT)
	./$(CLIENT)

demo: all
	./$(SERVER) & \
	sleep 1; \
	echo "client1: hello" | ./$(CLIENT); \
	echo "client2: bye" | ./$(CLIENT); \
	kill $$! 2>/dev/null || true

clean:
	rm -f $(SERVER) $(CLIENT)
	rm -f /tmp/uds_upper.sock
