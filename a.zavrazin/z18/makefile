CC = gcc
CFLAGS = -Wall -Wextra
TARGET = task.out
SRC = task.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean