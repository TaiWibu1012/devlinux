CC = gcc
# SỬA LỖI BUILD SYSTEM: Thêm đầy đủ flag -std=gnu99 và -g theo chuẩn doanh nghiệp
CFLAGS = -Wall -Wextra -O2 -std=gnu99 -g
LIBS = -pthread

TARGET = booking
SRCS = booking.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean