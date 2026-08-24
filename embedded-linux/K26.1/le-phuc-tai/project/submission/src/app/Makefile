# ==============================================================================
# Makefile for Smart Weather Alarm Clock (Userspace Application)
# Enterprise Embedded Linux Standard
# ==============================================================================

CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Wextra -O2 -pipe -I../../include -I.
LDFLAGS ?= -pthread -lrt -lm

# Target executable name
TARGET = smartclock

# Source files
SRCS = main.c \
       clock_screen.c \
       weather_screen.c \
       alarm_manager.c \
       smartconfig.c \
       webserver.c \
       ssd1306_oled.c

# Object files
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "=================================================="
	@echo "  Built Userspace Application: $(TARGET) [OK]"
	@echo "=================================================="

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned Userspace build artifacts."