#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define MAX_TRANSACTIONS  5   /* Số vòng lặp tối đa của phiên làm việc */
#define CRITICAL_TIME_SEC 3   /* Thời gian xử lý trong vùng an toàn (Critical Section) */
#define IDLE_TIME_SEC     3   /* Thời gian nghỉ trong vùng rỗi (Idle Section) */

int main(void) {
    setbuf(stdout, NULL);

    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);

    for (int i = 1; i <= MAX_TRANSACTIONS; i++) {
        /* --- GIAI ĐOẠN 1: VÙNG AN TOÀN (CRITICAL SECTION) --- */
        /* Chặn tín hiệu SIGINT để bảo vệ tiến trình ghi dữ liệu, lưu cấu hình cũ vào old_set */
        if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
            perror("CRITICAL: Failed to apply signal mask");
            return EXIT_FAILURE;
        }

        printf("[SAFE] Writing transaction #%d ...\n", i);
        sleep(CRITICAL_TIME_SEC);
        printf("[SAFE] Transaction #%d committed.\n", i);

        /* Khôi phục lại đúng trạng thái mask cũ của hệ thống (Restore Mask) */
        if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
            perror("CRITICAL: Failed to restore signal mask");
            return EXIT_FAILURE;
        }

        /* --- GIAI ĐOẠN 2: VÙNG NGHỈ (IDLE SECTION) --- */
        /* Tại đây SIGINT không bị chặn, nếu người dùng ấn Ctrl+C chương trình sẽ sập theo default action */
        printf("[IDLE] Waiting for next transaction...\n");
        sleep(IDLE_TIME_SEC);
    }

    return EXIT_SUCCESS;
}