CC = gcc
CXX = g++
CFLAGS = -std=c99 -Wall -Wextra -Iinclude -g
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g
TEST_LIBS = -lgtest -lgtest_main -lpthread

TARGET = build/tel-gateway
TEST_TARGET = build/run_tests
OBJ_DIR = build/obj

# Gather all source files
SRC = $(wildcard src/*.c src/core/*.c src/log/*.c src/system/*.c src/network/*.c)
CORE_SRC = $(filter-out src/main.c, $(SRC))
CORE_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CORE_SRC))

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Rule to compile pure C files safely using GCC to avoid C++ malloc errors
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	@echo "========================================="
	@echo "        Running GoogleTest Suite        "
	@echo "========================================="
	./$(TEST_TARGET)

$(TEST_TARGET): tests/unit/test_gateway.cpp $(CORE_OBJS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $^ $(TEST_LIBS) -o $(TEST_TARGET)

clean:
	rm -rf build

.PHONY: all clean test
