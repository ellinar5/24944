#!/bin/bash
set -e

SERVER="./server"
CLIENT="./client"
SOCKET_PATH="/tmp/uppercase_socket_nbezborodov"

cleanup() {
  echo
  echo "Stopping..."
  kill "$SERVER_PID" "$C1_PID" "$C2_PID" "$C3_PID" "$C4_PID" "$C5_PID" 2>/dev/null || true
  wait "$SERVER_PID" "$C1_PID" "$C2_PID" "$C3_PID" "$C4_PID" "$C5_PID" 2>/dev/null || true
  rm -f "$SOCKET_PATH"
  exit 0
}

trap cleanup INT TERM

rm -f "$SOCKET_PATH"

echo "Starting server..."
$SERVER &
SERVER_PID=$!

# wait for socket to appear
for i in {1..50}; do
  [ -S "$SOCKET_PATH" ] && break
  sleep 0.1
done

if [ ! -S "$SOCKET_PATH" ]; then
  echo "Error: server did not create socket: $SOCKET_PATH"
  kill "$SERVER_PID" 2>/dev/null || true
  exit 1
fi

echo "Starting clients..."

( while true; do
    echo "first: Hello World"
    echo "first: aBcDeeqASZ"
    sleep 0.45
  done ) | $CLIENT & C1_PID=$!

( while true; do
    echo "second: second client"
    echo "second: num and words"
    sleep 0.70
  done ) | $CLIENT & C2_PID=$!

( while true; do
    echo "third: too fast"
    echo "third: jumps over the lazy dog"
    sleep 0.20
  done ) | $CLIENT & C3_PID=$!

( while true; do
    echo "fourth: oZ"
    echo "fourth: i LoVe UnIx"
    sleep 0.35
  done ) | $CLIENT & C4_PID=$!

( while true; do
    echo "fifth: a lit bit slower"
    echo "fifth: we're finish"
    sleep 0.60
  done ) | $CLIENT & C5_PID=$!

echo "Running. Press Ctrl+C to stop."
wait
