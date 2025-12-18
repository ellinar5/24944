all:
	gcc server.c -o server
	gcc client.c -o client
	rm -f /tmp/my_socket
	./server &
	sleep 2
	./client &
	sleep 3
	pkill server

clean:
	rm -f server client /tmp/my_socket
	pkill server 2>/dev/null || true
	pkill client 2>/dev/null || true