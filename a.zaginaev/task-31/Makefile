all:
    gcc -o result31 31.c
    ./result31

    gcc -o client client.c
    i=1; while [ $i -le 15 ]; do echo "hello from client $i" | ./client $i & i=$((i+1)); done; wait
    
