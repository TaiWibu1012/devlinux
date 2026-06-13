CC = gcc
CFLAGS = -Wall -Wextra -O2

TARGET = manager
SRCS = manager.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 