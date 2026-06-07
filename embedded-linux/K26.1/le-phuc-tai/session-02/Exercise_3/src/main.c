#include <stdio.h>
#include "calc.h"
#include "logger.h"

int main() {
    log_info("--- Ứng dụng bắt đầu khởi chạy ---");

    double a = 12.5, b = 4.0;
    int err = 0;

    // Thực hiện tính toán và ghi log quy trình
    double res_add = add(a, b);
    log_info("Thực hiện phép tính CỘNG thành công.");

    double res_sub = subtract(a, b);
    log_info("Thực hiện phép tính TRỪ thành công.");

    double res_mul = multiply(a, b);
    log_info("Thực hiện phép tính NHÂN thành công.");

    double res_div = divide(a, b, &err);
    log_info("Thực hiện phép tính CHIA thành công.");

    divide(a, 0.0, &err);
    if (err) {
        log_error("Phát hiện hành vi nguy hiểm: Chia cho số 0!");
    }

    printf("%.2f + %.2f = %.2f\n", a, b, res_add);
    printf("%.2f - %.2f = %.2f\n", a, b, res_sub);
    printf("%.2f * %.2f = %.2f\n", a, b, res_mul);
    printf("%.2f / %.2f = %.2f\n", a, b, res_div);
    printf("%.2f / 0.00 = LỖI (Đã ghi nhận vào file app.log)\n", a);

    log_info("--- Ứng dụng kết thúc an toàn ---");
    return 0;
}