#!/bin/bash
SERVER="./server"
CLIENT="./client"
SOCKET_PATH="/tmp/uppercase_socket"

cleanup() {
  echo "Stopping..."
  kill "$SERVER_PID" "$C1_PID" "$C2_PID" "$C3_PID" "$C4_PID" 2>/dev/null
  wait "$SERVER_PID" "$C1_PID" "$C2_PID" "$C3_PID" "$C4_PID" 2>/dev/null
  rm -f "$SOCKET_PATH"
  exit 0
}

trap cleanup INT TERM

echo "Starting server..."
$SERVER &
SERVER_PID=$!

for i in {1..50}; do
  [ -S "$SOCKET_PATH" ] && break
  sleep 0.1
done

if [ ! -S "$SOCKET_PATH" ]; then
  echo "Error: socket not created"
  kill "$SERVER_PID" 2>/dev/null
  exit 1
fi

echo "Starting clients..."

( while true; do
    echo "first: Hello, Server!"
    echo "first: select -> should become UPPER"
    sleep 0.5
  done ) | $CLIENT & C1_PID=$!

( while true; do
    echo "second: second line"
    echo "second: 1qaz2wsx3edc"
    sleep 0.7
  done ) | $CLIENT & C2_PID=$!

( while true; do
    echo "third: operating sys"
    echo "third: sockets/select test"
    sleep 0.2
  done ) | $CLIENT & C3_PID=$!

( while true; do
    echo "fourth: last client"
    echo "fourth: can I have + a cup of COFFE"
    sleep 0.4
  done ) | $CLIENT & C4_PID=$!

echo "Running. Ctrl+C to stop."
wait
