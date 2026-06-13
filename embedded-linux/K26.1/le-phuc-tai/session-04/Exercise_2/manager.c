#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define INPUT_MAX_LEN 100
#define DB_FILE "students.txt"
#define BIN_SEARCHER "./searcher"

/* Khai báo mảng môi trường toàn cục từ glibc */
extern char **environ;

int main(void) {
    char input_buf[INPUT_MAX_LEN];

    printf("\n=============================================\n");
    printf("   STUDENT LOOKUP SYSTEM — MANAGER\n");
    printf("   (fork + execve | file: %s)\n", DB_FILE);
    printf("=============================================\n");
    printf("[MANAGER] PID: %d\n", getpid());

    while (1) {
        printf("Enter student ID ('quit' to exit).\n");
        printf("---------------------------------------------\n");
        printf("Student ID: ");
        fflush(stdout);

        /* Đọc dữ liệu đầu vào một cách an toàn từ stdin (Tránh Buffer Overflow) */
        if (fgets(input_buf, sizeof(input_buf), stdin) == NULL) {
            break;
        }

        /* Loại bỏ ký tự xuống dòng '\n' */
        input_buf[strcspn(input_buf, "\r\n")] = '\0';

        /* Trường hợp chuỗi rỗng do người dùng chỉ ấn Enter */
        if (strlen(input_buf) == 0) {
            continue;
        }

        /* Kiểm tra điều kiện thoát */
        if (strcmp(input_buf, "quit") == 0) {
            printf("[MANAGER] Exiting. Goodbye!\n");
            break;
        }

        /* Khởi tạo tiến trình con để gánh vác tác vụ tìm kiếm độc lập */
        pid_t pid = fork();

        if (pid < 0) {
            perror("[MANAGER] ERROR: Fork failed");
            continue;
        } else if (pid == 0) {
            /* Nhánh Tiến trình con: Thiết lập danh sách đối số cho execve */
            /* Theo chuẩn POSIX, args[0] phải là đường dẫn file thực thi */
            char *args[] = {BIN_SEARCHER, input_buf, DB_FILE, NULL};

            execve(args[0], args, environ);

            /* --- ĐOẠN MÃ DƯỚI ĐÂY LÀ ĐOẠN MÃ KHÔNG TƯỞNG (UNREACHABLE CODE) ---
             * Lý do: Nếu hàm execve() thực thi thành công, nó sẽ thay thế toàn bộ 
             * không gian địa chỉ, phân đoạn code, stack, heap của tiến trình con bằng 
             * file thực thi "searcher". Tiến trình con sẽ chạy mã mới hoàn toàn và không 
             * bao giờ quay lại dòng này. Dòng lệnh perror này CHỈ chạy khi execve thất bại 
             * (ví dụ: không tìm thấy file './searcher' hoặc file không có quyền thực thi).
             */
            perror("[MANAGER] CRITICAL ERROR: execve() failed");
            exit(2);
        } else {
            /* Nhánh Tiến trình cha: Chờ tiến trình con thực thi xong tác vụ */
            int status = 0;
            printf("\n[MANAGER] fork() → child PID: %d\n", pid);
            printf("[MANAGER] Waiting for child (waitpid)...\n\n");

            if (waitpid(pid, &status, 0) == -1) {
                perror("[MANAGER] ERROR: waitpid failed");
                continue;
            }

            /* Đọc và phân tích mã lỗi trả về từ khế ước của tiến trình con */
            if (WIFEXITED(status)) {
                int child_exit_code = WEXITSTATUS(status);
                printf("[MANAGER] Child (PID %d) exited. code=%d → ", pid, child_exit_code);
                
                switch (child_exit_code) {
                    case 0:
                        printf("Found\n");
                        break;
                    case 1:
                        printf("Not found\n");
                        break;
                    case 2:
                        printf("System/File error occurred in searcher\n");
                        break;
                    default:
                        printf("Unknown exit status\n");
                        break;
                }
            } else {
                printf("[MANAGER] Child (PID %d) terminated abnormally\n", pid);
            }
            printf("---------------------------------------------\n");
        }
    }

    return EXIT_SUCCESS;
} 