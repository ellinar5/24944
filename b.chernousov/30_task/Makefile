all:
	gcc server.c -o server
	gcc client.c -o client
	rm -f /tmp/my_socket
	./server &
	sleep 1
	./client test.txt
	sleep 1
	pkill server