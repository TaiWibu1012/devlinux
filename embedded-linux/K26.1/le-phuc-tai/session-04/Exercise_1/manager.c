#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_ORDERS 3
#define ORDER_NAME_LEN 50

/* Định nghĩa cấu trúc dữ liệu theo nghiệp vụ */
typedef struct {
    int id;
    char name[ORDER_NAME_LEN];
    int quantity;
    double unit_price; /* Dùng double để tránh tràn số trong thực tế */
} Order;

/* Khai báo prototype của hàm */
void process_order(Order order);
void print_formatted_revenue(double revenue);

int main(void) {
    /* Hardcode dữ liệu đầu vào theo đặc tả yêu cầu */
    Order orders[MAX_ORDERS] = {
        {1, "Backpack", 2, 350000.0},
        {2, "Shoes",    1, 500000.0},
        {3, "Hat",      3, 120000.0}
    };

    pid_t pids[MAX_ORDERS] = {0};
    int total_spawned = 0;

    printf("\n===================================================\n");
    printf("   ORDER PROCESSING SYSTEM — MANAGER (fork+wait)\n");
    printf("===================================================\n");
    printf("[MANAGER] PID: %d — spawning %d child processes...\n\n", getpid(), MAX_ORDERS);

    /* LOOP 1: Khởi tạo tất cả tiến trình con xử lý song song (Concurrency) */
    for (int i = 0; i < MAX_ORDERS; i++) {
        /* Tiêu chuẩn bắt buộc: Xóa bộ đệm stdout trước khi fork để tránh nhân bản log */
        fflush(stdout);

        pid_t pid = fork(); 

        if (pid < 0) {
            perror("[MANAGER] ERROR: fork() failed");
            /* Lập trình phòng thủ: Tiếp tục xử lý các tiến trình đã tạo thành công, không crash */
            continue;
        } else if (pid == 0) {
            /* Nhánh của Tiến trình con (Child process) */
            process_order(orders[i]);
            /* Kết thúc tiến trình con một cách an toàn, trả về code thành công 0 */
            exit(EXIT_SUCCESS);
        } else {
            /* Nhánh của Tiến trình cha (Parent process) */
            pids[i] = pid;
            total_spawned++;
            printf("[MANAGER] fork() order #%d → child PID: %d\n", orders[i].id, pid);
        }
    }

    printf("[MANAGER] All %d children spawned. Starting waitpid()...\n\n", total_spawned);
    printf("--- [child output order may interleave — this is normal] ---\n\n");

    /* LOOP 2: Đồng bộ và thu gom trạng thái của các tiến trình con */
    int successful_orders = 0;
    int failed_orders = 0;
    double total_revenue = 0.0;

    for (int i = 0; i < MAX_ORDERS; i++) {
        /* Bỏ qua nếu tiến trình đó fork thất bại */
        if (pids[i] == 0) {
            failed_orders++;
            continue;
        }

        int status = 0;
        /* Chờ đích danh child PID theo đúng thứ tự mảng để đảm bảo tính nhất quán của báo cáo */
        pid_t waited_pid = waitpid(pids[i], &status, 0);

        if (waited_pid == -1) {
            perror("[MANAGER] ERROR: waitpid() failed");
            failed_orders++;
            continue;
        }

        printf("[MANAGER] waitpid(%d) — order #%d: ", pids[i], orders[i].id);

        /* Kiểm tra trạng thái kết thúc của tiến trình con */
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("exit code=%d", exit_code);

            if (exit_code == EXIT_SUCCESS) {
                printf(" → SUCCESS\n");
                successful_orders++;
                total_revenue += (orders[i].quantity * orders[i].unit_price);
            } else {
                printf(" → FAILED (Sub-logic error)\n");
                failed_orders++;
            }
        } else {
            printf(" → TERMINATED ABNORMALLY\n");
            failed_orders++;
        }
    }

    /* Xuất báo cáo tổng hợp (Summary Report) */
    printf("\n================= SUMMARY =================\n");
    printf("  Total orders    : %d\n", MAX_ORDERS);
    printf("  Successful      : %d\n", successful_orders);
    printf("  Failed          : %d\n", failed_orders);
    printf("  Total revenue   : ");
    print_formatted_revenue(total_revenue);
    printf("\n===========================================\n");

    return EXIT_SUCCESS;
}

/**
 * @brief Mô phỏng logic xử lý đơn hàng tại tiến trình con.
 */
void process_order(Order order) {
    double total = order.quantity * order.unit_price;
    printf("[CHILD-%d] PID: %d | PPID: %d\n", order.id, getpid(), getppid());
    printf("[CHILD-%d] %s x%d — Total: %.0f VND\n", order.id, order.name, order.quantity, total);
    printf("[CHILD-%d] Processing... (sleep 2s)\n\n", order.id);
    
    /* Mô phỏng tác vụ I/O hoặc tính toán tốn thời gian */
    sleep(2);
}

/**
 * @brief Hàm hỗ trợ format tiền tệ có dấu phẩy phân cách hàng nghìn (Chuẩn doanh nghiệp).
 */
void print_formatted_revenue(double revenue) {
    char buffer[100];
    int len = snprintf(buffer, sizeof(buffer), "%.0f", revenue);
    
    /* Chèn dấu phẩy phân cách hàng nghìn */
    for (int i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 3 == 0) {
            printf(",");
        }
        printf("%c", buffer[i]);
    }
    printf(" VND");
}