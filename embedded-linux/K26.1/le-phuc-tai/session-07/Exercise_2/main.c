#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INIT_BLOCK_TIME_SEC 5   /* Thời gian tiến trình cha chặn tín hiệu khi khởi tạo */
#define WORKER_DELAY_SEC    2   /* Thời gian tiến trình con chuẩn bị trước khi phát tín hiệu */
#define WORKER_EXIT_CODE    7   /* Mã kết thúc chỉ định của tiến trình con */

/**
 * @brief Bộ xử lý tín hiệu SIGUSR1 khi nhận báo cáo từ Worker
 */
void handle_sigusr1(int sig) {
    (void)sig;
    const char msg[] = "[GATEWAY] Worker reported READY signal received\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
}

int main(void) {
    setbuf(stdout, NULL);

    /* Thiết lập handler cho SIGUSR1 từ trước khi fork để tránh race condition */
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("CRITICAL: Gateway failed to register SIGUSR1");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("CRITICAL: Process duplication via fork() failed");
        return EXIT_FAILURE;
    } 
    else if (pid == 0) {
        /* --- TIẾN TRÌNH CON (WORKER) --- */
        sleep(WORKER_DELAY_SEC);
        
        /* Gửi tín hiệu phần mềm thông báo trạng thái sẵn sàng lên tiến trình cha */
        if (kill(getppid(), SIGUSR1) < 0) {
            perror("[WORKER] ERROR: Failed to send signal to gateway");
            exit(EXIT_FAILURE);
        }
        
        printf("[WORKER] Sent READY signal to gateway\n");
        exit(WORKER_EXIT_CODE);
    } 
    else {
        /* --- TIẾN TRÌNH CHA (GATEWAY) --- */
        printf("[GATEWAY] Worker PID = %d\n", pid);

        sigset_t block_set;
        sigemptyset(&block_set);
        sigaddset(&block_set, SIGUSR1);

        /* Thực hiện đưa SIGUSR1 vào danh sách mặt nạ chặn (Signal Mask) */
        if (sigprocmask(SIG_BLOCK, &block_set, NULL) < 0) {
            perror("[GATEWAY] CRITICAL: Failed to block SIGUSR1");
            return EXIT_FAILURE;
        }

        /* Mô phỏng giai đoạn khởi tạo hệ thống không muốn bị làm phiền */
        sleep(INIT_BLOCK_TIME_SEC);

        /* Gỡ bỏ trạng thái chặn, các tín hiệu pending từ worker gửi ở giây thứ 2 sẽ được giải phóng tại đây */
        if (sigprocmask(SIG_UNBLOCK, &block_set, NULL) < 0) {
            perror("[GATEWAY] CRITICAL: Failed to unblock SIGUSR1");
            return EXIT_FAILURE;
        }

        int status = 0;
        if (wait(&status) < 0) {
            perror("[GATEWAY] ERROR: wait() execution failed");
            return EXIT_FAILURE;
        }

        /* Phân tích trạng thái kết thúc của tiến trình con */
        if (WIFEXITED(status)) {
            printf("[GATEWAY] Worker exited with code %d\n", WEXITSTATUS(status));
        } else {
            printf("[GATEWAY] Worker terminated abnormally\n");
        }
    }

    return EXIT_SUCCESS;
}