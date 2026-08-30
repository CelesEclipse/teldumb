CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Iinclude -g

TARGET = build/tel-gateway
SRC = $(wildcard src/*.c src/core/*.c src/log/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf build

.PHONY: all clean
