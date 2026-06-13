#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX_LEN 256
#define GRADE_LEN 20

/* Định nghĩa các Exit Code theo thiết kế hệ thống */
#define EXIT_FOUND        0
#define EXIT_NOT_FOUND    1
#define EXIT_SYS_ERROR    2

void get_grade_classification(float gpa, char *grade_out);

int main(int argc, char *argv[]) {
    /* Kiểm tra tính hợp lệ của đối số dòng lệnh đầu vào */
    if (argc < 3) {
        fprintf(stderr, "[SEARCHER] ERROR: Missing arguments. Usage: %s <student_id> <data_file>\n", argv[0]);
        exit(EXIT_SYS_ERROR);
    }

    char *target_id = argv[1];
    char *file_path = argv[2];

    printf("[SEARCHER] PID: %d | PPID: %d\n", getpid(), getppid());
    printf("[SEARCHER] Searching for \"%s\" in %s...\n\n", target_id, file_path);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("[SEARCHER] ERROR: Cannot open database file");
        exit(EXIT_SYS_ERROR);
    }

    char line[LINE_MAX_LEN];
    int is_found = 0;

    /* Đọc file tuần tự theo từng dòng dòng để tối ưu dung lượng RAM (Stream Processing) */
    while (fgets(line, sizeof(line), file) != NULL) {
        /* Xóa ký tự xuống dòng dính ở cuối dòng nếu có */
        line[strcspn(line, "\r\n")] = '\0';

        /* Sao chép dòng dữ liệu để tránh làm hỏng bộ đệm gốc khi phân tách chuỗi */
        char line_copy[LINE_MAX_LEN];
        strncpy(line_copy, line, sizeof(line_copy));

        /* Phân tách chuỗi bằng token '|' (Lập trình phòng thủ: Kiểm tra NULL từng token) */
        char *id = strtok(line_copy, "|");
        char *name = strtok(NULL, "|");
        char *class_name = strtok(NULL, "|");
        char *gpa_str = strtok(NULL, "|");

        if (!id || !name || !class_name || !gpa_str) {
            /* Bỏ qua các dòng dữ liệu lỗi định dạng trong file thực tế */
            continue; 
        }

        /* So khớp ID sinh viên cần tìm */
        if (strcmp(id, target_id) == 0) {
            is_found = 1;
            float gpa = strtof(gpa_str, NULL);
            char grade[GRADE_LEN];
            
            get_grade_classification(gpa, grade);

            /* In kết quả chuẩn định dạng báo cáo */
            printf("========== SEARCH RESULT ==========\n");
            printf("  ID      : %s\n", id);
            printf("  Name    : %s\n", name);
            printf("  Class   : %s\n", class_name);
            printf("  GPA     : %.1f\n", gpa);
            printf("  Grade   : %s\n", grade);
            printf("====================================\n\n");
            break;
        }
    }

    fclose(file);

    if (is_found) {
        exit(EXIT_FOUND);
    } else {
        printf("[SEARCHER] No student found with ID: %s\n\n", target_id);
        exit(EXIT_NOT_FOUND);
    }
}

/**
 * @brief Phân loại học lực dựa trên điểm GPA theo quy chuẩn doanh nghiệp.
 */
void get_grade_classification(float gpa, char *grade_out) {
    if (gpa >= 8.5f) {
        strcpy(grade_out, "Excellent");
    } else if (gpa >= 7.0f) {
        strcpy(grade_out, "Good");
    } else if (gpa >= 5.0f) {
        strcpy(grade_out, "Average");
    } else {
        strcpy(grade_out, "Poor");
    }
} 