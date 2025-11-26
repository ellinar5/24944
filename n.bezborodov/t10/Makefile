all: build test1 test2 test3 test4
build:
	gcc t10.c -o t10
test1:
	./t10 ls -l
test2:
	./t10 sleep 1
test3:
	-./t10 ls /nope
test4:
	-./t10 sh -c "kill -9 $$"