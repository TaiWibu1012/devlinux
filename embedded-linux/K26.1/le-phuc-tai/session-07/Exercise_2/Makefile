CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu99 -g

TARGET = iot_gateway
SRCS = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean