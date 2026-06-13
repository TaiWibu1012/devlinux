#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define QUEUE_CAPACITY 5        /* Sức chứa tối đa của hàng đợi máy in (Ring Buffer) */
#define NUM_PRODUCERS 3         /* Số lượng luồng văn phòng (Producers) đẩy tài liệu vào */
#define DOCS_PER_PRODUCER 3     /* Số lượng tài liệu mà mỗi luồng sản xuất bắt buộc phải nộp */
#define FILENAME_MAX_LEN 60     /* Giới hạn ký tự của tên file để tránh Buffer Overflow */

/* Cấu trúc tài liệu in */
typedef struct {
    int doc_id;
    char filename[FILENAME_MAX_LEN];
    int pages;
} Document;

/* Hàng đợi xoay vòng (Circular Ring Buffer) */
Document queue[QUEUE_CAPACITY];
int head = 0;
int tail = 0;
int count = 0;
int all_sent = 0; /* Cờ báo hiệu kết thúc giai đoạn nạp dữ liệu */

/* Biến thống kê báo cáo */
int total_submitted = 0;
int total_printed = 0;
int total_pages_printed = 0;

/* Các công cụ đồng bộ hóa POSIX */
pthread_mutex_t q_lock;
pthread_cond_t not_full;   /* Điều kiện báo cho Producer: Hàng đợi còn chỗ trống */
pthread_cond_t not_empty;  /* Điều kiện báo cho Printer: Hàng đợi có tài liệu */

/**
 * @brief Hàm xử lý của các Luồng Sản Xuất (Producers)
 */
void *producer_worker(void *arg) {
    int producer_id = *(int *)arg;
    
    for (int i = 1; i <= DOCS_PER_PRODUCER; i++) {
        Document doc;
        doc.doc_id = (producer_id * 100) + i; 
        doc.pages = (rand() % 20) + 1;       
        
        /* SỬA LỖI STRING SAFETY: Dùng snprintf an toàn tuyệt đối, không dùng strcpy */
        if (producer_id == 1) {
            snprintf(doc.filename, sizeof(doc.filename), "report_Q%d.pdf", i);
        } else if (producer_id == 2) {
            snprintf(doc.filename, sizeof(doc.filename), "contract_%d.pdf", i);
        } else {
            snprintf(doc.filename, sizeof(doc.filename), "invoice_%d.pdf", i);
        }

        if (pthread_mutex_lock(&q_lock) != 0) {
            fprintf(stderr, "[Producer %d] ERROR: Mutex lock failed\n", producer_id);
            pthread_exit(NULL);
        }

        /* LẬP TRÌNH PHÒNG THỦ: Bắt buộc dùng vòng lặp WHILE để chống Spurious Wakeup */
        while (count == QUEUE_CAPACITY) {
            printf("[Producer %d] Queue full — waiting...\n", producer_id);
            if (pthread_cond_wait(&not_full, &q_lock) != 0) {
                fprintf(stderr, "[Producer %d] ERROR: Condition wait failed\n", producer_id);
                pthread_mutex_unlock(&q_lock);
                pthread_exit(NULL);
            }
        }

        /* Thêm tài liệu vào vị trí đuôi (Tail) của Ring Buffer */
        queue[tail] = doc;
        tail = (tail + 1) % QUEUE_CAPACITY;
        count++;
        total_submitted++;

        printf("[Producer %d] Submitting: %s (%d pages) — queue: %d/%d\n",
               producer_id, doc.filename, doc.pages, count, QUEUE_CAPACITY);

        /* Kiểm tra return value khi phát tín hiệu */
        if (pthread_cond_signal(&not_empty) != 0) {
            fprintf(stderr, "[Producer %d] ERROR: Condition signal failed\n", producer_id);
        }

        if (pthread_mutex_unlock(&q_lock) != 0) {
            fprintf(stderr, "[Producer %d] ERROR: Mutex unlock failed\n", producer_id);
        }
        
        usleep((rand() % 100 + 50) * 1000); 
    }

    pthread_exit(NULL);
}

/**
 * @brief Hàm xử lý của Luồng Máy In (Printer/Consumer)
 */
void *printer_worker(void *arg) {
    (void)arg; 

    while (1) {
        if (pthread_mutex_lock(&q_lock) != 0) {
            fprintf(stderr, "[Printer] ERROR: Mutex lock failed\n");
            pthread_exit(NULL);
        }

        /* Chờ cho đến khi có hàng hoặc toàn bộ luồng đẩy đã xong việc */
        while (count == 0 && !all_sent) {
            if (pthread_cond_wait(&not_empty, &q_lock) != 0) {
                fprintf(stderr, "[Printer] ERROR: Condition wait failed\n");
                pthread_mutex_unlock(&q_lock);
                pthread_exit(NULL);
            }
        }

        /* Điều kiện thoát an toàn: Hàng đợi trống rỗng hoàn toàn và không còn ai gửi nữa */
        if (count == 0 && all_sent) {
            pthread_mutex_unlock(&q_lock);
            break;
        }

        /* Lấy tài liệu ra từ đầu (Head) của Ring Buffer */
        Document doc = queue[head];
        head = (head + 1) % QUEUE_CAPACITY;
        count--;

        total_printed++;
        total_pages_printed += doc.pages;

        printf("[Printer]    Printing:   %s (%d pages) — queue: %d/%d\n",
               doc.filename, doc.pages, count, QUEUE_CAPACITY);

        if (pthread_cond_signal(&not_full) != 0) {
            fprintf(stderr, "[Printer] ERROR: Condition signal failed\n");
        }

        if (pthread_mutex_unlock(&q_lock) != 0) {
            fprintf(stderr, "[Printer] ERROR: Mutex unlock failed\n");
        }

        sleep(1); /* Mô phỏng thời gian phần cứng máy in hoạt động thực tế */
    }

    printf("[Printer]    All documents printed. Exiting.\n");
    pthread_exit(NULL);
}

int main(void) {
    srand(time(NULL));

    pthread_t producers[NUM_PRODUCERS];
    pthread_t printer;
    int producer_ids[NUM_PRODUCERS];
    int producers_spawned[NUM_PRODUCERS] = {0}; /* Mảng kiểm tra an toàn luồng tránh crash khi join */
    int rc;

    printf("==============================================\n");
    printf("   OFFICE PRINT QUEUE (3 producers, 1 printer)\n");
    printf("   Queue capacity: %d documents\n", QUEUE_CAPACITY);
    printf("==============================================\n\n");

    /* Kiểm tra return value khi khởi tạo toàn bộ công cụ đồng bộ */
    if (pthread_mutex_init(&q_lock, NULL) != 0 ||
        pthread_cond_init(&not_full, NULL) != 0 ||
        pthread_cond_init(&not_empty, NULL) != 0) {
        fprintf(stderr, "CRITICAL ERROR: Synchronization primitives initialization failed\n");
        return EXIT_FAILURE;
    }

    /* Kích hoạt luồng tiêu thụ (Printer Consumer) */
    rc = pthread_create(&printer, NULL, printer_worker, NULL);
    if (rc != 0) {
        fprintf(stderr, "CRITICAL ERROR: Cannot create printer thread (code: %d)\n", rc);
        pthread_mutex_destroy(&q_lock);
        pthread_cond_destroy(&not_full);
        pthread_cond_destroy(&not_empty);
        return EXIT_FAILURE;
    }

    /* Kích hoạt các luồng tạo dữ liệu (Producers) */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i + 1;
        rc = pthread_create(&producers[i], NULL, producer_worker, &producer_ids[i]);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Cannot create producer thread %d (code: %d)\n", i + 1, rc);
            producers_spawned[i] = 0;
        } else {
            producers_spawned[i] = 1;
        }
    }

    /* Đợi toàn bộ các Producer đẩy xong hết số tài liệu */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        if (producers_spawned[i] == 1) {
            pthread_join(producers[i], NULL);
        }
    }

    /* GIAI ĐOẠN ĐỒNG BỘ CUỐI */
    pthread_mutex_lock(&q_lock);
    all_sent = 1; 
    pthread_cond_broadcast(&not_empty); /* Dùng broadcast đánh thức máy in an toàn */
    pthread_mutex_unlock(&q_lock);

    /* Đợi luồng máy in xử lý nốt phần việc tồn đọng và tự thoát */
    pthread_join(printer, NULL);

    printf("\n================ SUMMARY ================\n");
    printf("  Documents submitted : %d\n", total_submitted);
    printf("  Documents printed   : %d\n", total_printed);
    printf("  Total pages printed : %d\n", total_pages_printed);
    printf("=========================================\n");

    /* Dọn dẹp tài nguyên hệ thống phòng chống rò rỉ (Resource Leaks) và kiểm tra kỹ lỗi trả về */
    if (pthread_mutex_destroy(&q_lock) != 0 ||
        pthread_cond_destroy(&not_full) != 0 ||
        pthread_cond_destroy(&not_empty) != 0) {
        fprintf(stderr, "WARNING: Error occurred during resource destruction\n");
    }

    return EXIT_SUCCESS;
}

/* ============================================================================
 * COMMENT BLOCK GIẢI THÍCH (YÊU CẦU ĐẶC TẢ)
 * * 1. Tại sao "pthread_cond_wait()" bắt buộc phải đặt trong vòng lặp 'while' thay vì lệnh 'if'?
 * * Nếu dùng 'if', khi luồng thức dậy nó sẽ mặc định tin tưởng môi trường đã an toàn và thực thi luôn 
 * dòng lệnh nạp dữ liệu phía dưới mà không kiểm tra lại. Điều này dẫn tới thảm họa tràn hàng đợi nếu có 
 * một luồng khác đã nhanh chân hơn vào chiếm mất slot trống ngay trước đó. Vòng lặp 'while' ép buộc luồng 
 * sau khi thức dậy phải tái đánh giá lại điều kiện (re-evaluate predicate).
 * * 2. Hiện tượng thức tỉnh giả (Spurious Wakeup) là gì?
 * * Thức tỉnh giả là hiện tượng một luồng đang ngủ trong hàm pthread_cond_wait() bất ngờ bị đánh thức hoạt động 
 * trở lại bởi hệ điều hành, mặc dù trên thực tế chưa hề có bất kỳ tín hiệu signal/broadcast nào được phát ra. 
 * Nguyên nhân đến từ tầng sâu kiến trúc hạt nhân Linux (Kernel OS) nhằm tối ưu hiệu năng ngắt phần cứng. 
 * Lập trình viên không thể ngăn chặn nó, mà chỉ có thể phòng ngự bằng cách bọc hàm wait trong vòng lặp 'while'.
 * ============================================================================ */