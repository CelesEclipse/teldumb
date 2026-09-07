CC = gcc
CXX = g++
CFLAGS = -std=c99 -Wall -Wextra -Iinclude -g
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g
TEST_LIBS = -lgtest -lgtest_main -lpthread

TARGET = build/tel-gateway
OBJ_DIR = build/obj

# Test Targets & Binaries
UNIT_TEST_TARGET = build/run_unit_tests
INT_TEST_TARGET = build/run_integration_tests

# Gather all source files
SRC = $(wildcard src/*.c src/core/*.c src/log/*.c src/system/*.c src/network/*.c)
CORE_SRC = $(filter-out src/main.c, $(SRC))
CORE_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(CORE_SRC))

# Separate Test File Sources
UNIT_TEST_SRC = $(wildcard tests/unit/*.cpp)
INT_TEST_SRC = $(wildcard tests/integration/*.cpp)

.PHONY: all clean test integration

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Rule to compile pure C files safely using GCC to avoid C++ malloc errors
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---- UNIT TESTING ----
test: $(UNIT_TEST_TARGET)
	@echo "========================================="
	@echo "        Running Unit Test Suite        "
	@echo "========================================="
	./$(UNIT_TEST_TARGET)

$(UNIT_TEST_TARGET): $(UNIT_TEST_SRC) $(CORE_OBJS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $^ $(TEST_LIBS) -o $(UNIT_TEST_TARGET)

# ---- INTEGRATION TESTING ----
integration: $(INT_TEST_TARGET)
	@echo "========================================="
	@echo "     Running Integration Test Suite      "
	@echo "========================================="
	./$(INT_TEST_TARGET)

$(INT_TEST_TARGET): $(INT_TEST_SRC) $(CORE_OBJS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $^ $(TEST_LIBS) -o $(INT_TEST_TARGET)

clean:
	rm -rf build
