CC = gcc
CFLAGS = -Wall -Iinclude -fPIC
LDFLAGS = -Llib -lcalc -llogger

SRCS = $(wildcard src/*.c)

OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

STATIC_OBJ = $(filter %calc.o, $(OBJS))
SHARED_OBJ = $(filter %logger.o, $(OBJS))
MAIN_OBJ   = $(filter %main.o, $(OBJS))

EXEC = bin/main
STATIC_LIB = lib/libcalc.a
SHARED_LIB = lib/liblogger.so

# Target theo yêu cầu

all: $(STATIC_LIB) $(SHARED_LIB) $(EXEC)

libs: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

# Quy tắc Mẫu (Pattern Rule) để biên dịch các file .c thành .o
obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# Tạo thư viện tĩnh từ object tương ứng
$(STATIC_LIB): $(STATIC_OBJ)
	@mkdir -p lib
	ar rcs $@ $(STATIC_OBJ)

# Tạo thư viện chia sẻ từ object tương ứng
$(SHARED_LIB): $(SHARED_OBJ)
	@mkdir -p lib
	$(CC) -shared -o $@ $(SHARED_OBJ)

# Biên dịch chương trình chính kết hợp liên kết cả 2 loại thư viện
$(EXEC): $(MAIN_OBJ) $(STATIC_LIB) $(SHARED_LIB)
	@mkdir -p bin
	$(CC) $(MAIN_OBJ) $(LDFLAGS) -o $@

# Target chạy chương trình nhanh kèm nạp biến đường dẫn thư viện .so
run: all
	@echo "Thực thi chương trình tổng hợp"
	LD_LIBRARY_PATH=./lib ./$(EXEC)

clean:
	rm -rf obj lib bin app.log

# Xóa sạch và xây dựng lại từ đầu
rebuild: clean all

.PHONY: all libs static shared run clean rebuild