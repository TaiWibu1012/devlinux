#include <stdio.h>
#include <string.h>
#include "stringutils.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Lỗi: Thiếu tham số chuỗi đầu vào!\n");
        return 1;
    }

    char buffer[1024];
    strncpy(buffer, argv[1], sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; 

    printf("Chuỗi gốc ban đầu: '%s'\n", buffer);

    int len = count_chars(buffer);
    printf("1. Số lượng ký tự: %d\n", len);

    to_uppercase(buffer);
    printf("2. Sau khi viết hoa: %s\n", buffer);

    reverse_string(buffer);
    printf("3. Sau khi đảo ngược: %s\n", buffer);

    return 0;
}