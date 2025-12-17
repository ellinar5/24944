# Makefile for task31

all: srv clt

srv: task31_server.c
	gcc -std=c99 task31_server.c -o srv

clt: task31_client.c
	gcc -std=c99 task31_client.c -o clt

clean:
	rm -f srv clt ./socket