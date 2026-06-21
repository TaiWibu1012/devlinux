#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BASE_TEMP 25        /* Nhiệt độ cơ sở dùng để giả lập */
#define TEMP_MODULO 15      /* Hệ số chia lấy dư để xoay vòng giá trị nhiệt độ */

/* Biến đếm số lần đọc dữ liệu, dùng volatile sig_atomic_t để an toàn bất đồng bộ */
volatile sig_atomic_t g_reading_count = 0;

/**
 * @brief Bộ xử lý tín hiệu SIGINT (Ctrl+C)
 */
void handle_sigint(int sig) {
    (void)sig;
    /* Sử dụng write() thay vì printf() vì write() là hàm Async-Signal-Safe */
    const char msg[] = "[WARN] Received SIGINT, ignoring...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

/**
 * @brief Bộ xử lý tín hiệu kết thúc hệ thống (SIGTERM)
 */
void handle_sigterm(int sig) {
    (void)sig;
    const char msg[] = "[INFO] Received SIGTERM, shutting down gracefully...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    exit(0); /* Kết thúc chương trình với mã thành công theo đặc tả yêu cầu */
}

/**
 * @brief Bộ xử lý tín hiệu yêu cầu xuất báo cáo (SIGUSR1)
 */
void handle_sigusr1(int sig) {
    (void)sig;
    char buf[128] = "[REPORT] Total readings so far: ";
    int len = 32;
    int n = g_reading_count;
    char num_buf[20];
    int i = 0;
    
    /* Chuyển đổi số nguyên sang chuỗi một cách an toàn bên trong signal handler */
    if (n == 0) {
        num_buf[i++] = '0';
    } else {
        while (n > 0 && i < 20) {
            num_buf[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    while (i > 0) {
        buf[len++] = num_buf[--i];
    }
    buf[len++] = '\n';
    write(STDOUT_FILENO, buf, len);
}

int main(void) {
    /* Tắt bộ đệm dòng để log phân phát trực tiếp ra terminal */
    setbuf(stdout, NULL);

    /* Đăng ký các signal handler và kiểm tra nghiêm ngặt lỗi trả về */
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("CRITICAL: Failed to register SIGINT");
        return EXIT_FAILURE;
    }
    if (signal(SIGTERM, handle_sigterm) == SIG_ERR) {
        perror("CRITICAL: Failed to register SIGTERM");
        return EXIT_FAILURE;
    }
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("CRITICAL: Failed to register SIGUSR1");
        return EXIT_FAILURE;
    }

    while (1) {
        int temp = BASE_TEMP + (g_reading_count % TEMP_MODULO);
        printf("[INFO] Sensor reading #%d: temperature=%d\n", g_reading_count, temp);
        g_reading_count++;
        sleep(1);
    }

    return EXIT_SUCCESS;
}