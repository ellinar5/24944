all:
	gcc -std=c99 task32_server.c -o srv -lsocket
	gcc -std=c99 task32_client.c -o clt -lsocket

clean:
	rm -f srv clt

run_server:
	./srv

run_client:
	./clt 1 & ./clt 2