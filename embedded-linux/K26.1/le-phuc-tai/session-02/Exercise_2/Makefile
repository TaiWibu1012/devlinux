CC = gcc
CFLAGS = -Wall -Iinclude
SO_FLAGS = -shared
PIC_FLAG = -fPIC

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

LIB_NAME = libstringutils.so
SHARED_LIB = $(BIN_DIR)/$(LIB_NAME)
EXEC = $(BIN_DIR)/main

# Target mặc định: Build cả file .so và file chạy
all: $(SHARED_LIB) $(EXEC)

$(BUILD_DIR)/stringutils.o: $(SRC_DIR)/stringutils.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PIC_FLAG) -c $< -o $@

$(SHARED_LIB): $(BUILD_DIR)/stringutils.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(SO_FLAGS) $< -o $@

# Bước 3: Biên dịch main.c thành main.o
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC): $(BUILD_DIR)/main.o $(SHARED_LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(BUILD_DIR)/main.o -L$(BIN_DIR) -lstringutils -o $@

# Target dùng để chạy thử với một chuỗi mẫu
run: $(EXEC)
	LD_LIBRARY_PATH=./$(BIN_DIR) ./$(EXEC) "hello linux"

# Target dọn dẹp dự án
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all run clean