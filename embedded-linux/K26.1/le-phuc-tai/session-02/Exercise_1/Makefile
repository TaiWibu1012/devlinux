CC = gcc
CFLAGS = -Wall -Iinclude

SRC_DIR = src
APP_DIR = app
OBJ_DIR = obj
LIB_DIR = lib

# Tên file đầu ra
LIB_NAME = libmathutils.a
STATIC_LIB = $(LIB_DIR)/$(LIB_NAME)
EXEC = main

all: $(EXEC)

$(OBJ_DIR)/mathutils.o: $(SRC_DIR)/mathutils.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(STATIC_LIB): $(OBJ_DIR)/mathutils.o
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

$(EXEC): $(APP_DIR)/main.c $(STATIC_LIB)
	$(CC) $(CFLAGS) $< -L$(LIB_DIR) -lmathutils -o $@

run: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(EXEC)

.PHONY: all run clean