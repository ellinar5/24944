all:
	gcc server.c -o server
	gcc client.c -o client
	./server & sleep 0.1; for i in $$(seq 1 100); do ./client $$i & done; wait
