#include <stdio.h>
#include "mathutils.h"

int main() {
    int x, y, n;
    
    printf("Nhập vào 2 số nguyên : ");
    if (scanf("%d %d", &x, &y) != 2) {
        printf("Dữ liệu nhập không hợp lệ!\n");
        return 1;
    }
    printf("Kết quả %d + %d = %d\n", x, y, add(x, y));
    printf("Kết quả %d - %d = %d\n", x, y, subtract(x, y));

    // Nhập dữ liệu để test hàm giai thừa
    printf("\nNhập một số nguyên để tính giai thừa: ");
    if (scanf("%d", &n) != 1) {
        printf("Dữ liệu nhập không hợp lệ!\n");
        return 1;
    }
    printf("Giai thừa của %d là: %lld\n", n, factorial(n));

    return 0;
}