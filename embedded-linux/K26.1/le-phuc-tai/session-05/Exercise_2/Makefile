CC = gcc
# SỬA LỖI BUILD SYSTEM: CFLAGS bổ sung đầy đủ flag debug -g và tiêu chuẩn -std=gnu99
CFLAGS = -Wall -Wextra -O2 -std=gnu99 -g
LIBS = -pthread

TARGET = print_queue
SRCS = print_queue.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean