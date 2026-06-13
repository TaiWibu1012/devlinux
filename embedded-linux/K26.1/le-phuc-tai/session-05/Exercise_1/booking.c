#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_AGENTS 5        /* Số lượng luồng đại lý chạy song song xử lý đặt vé */
#define MAX_SEATS 10        /* Tổng số ghế trống tối đa trên chuyến xe */
#define NAME_MAX_LEN 50     /* Giới hạn độ dài chuỗi tên khách hàng để tránh buffer overflow */

/* Cấu trúc dữ liệu yêu cầu đặt vé */
typedef struct {
    int agent_id;
    char customer[NAME_MAX_LEN];
    int seats_wanted;
} BookingRequest;

/* Biến toàn cục (Shared Resources) */
int seats_available = MAX_SEATS;
int seats_sold = 0;
int failed_bookings = 0;

/* Mutex bảo vệ Critical Section */
pthread_mutex_t seat_lock;

/**
 * @brief Hàm Worker cho từng Agent Thread đảm nhận
 */
void *agent_worker(void *arg) {
    BookingRequest *req = (BookingRequest *)arg;
    
    /* Lấy Thread ID và ép kiểu an toàn sang unsigned long để in bằng %lu */
    unsigned long tid = (unsigned long)pthread_self();
    
    printf("[Agent %d | TID %lu] Booking %d %s for %s...\n", 
           req->agent_id, tid, req->seats_wanted, 
           (req->seats_wanted > 1) ? "seats" : "seat", req->customer);
    
    /* Mô phỏng môi trường chạy song song thực tế */
    sleep(1);

    /* --- BẮT ĐẦU CRITICAL SECTION --- */
    /* LẬP TRÌNH PHÒNG THỦ: Kiểm tra nghiêm ngặt giá trị trả về của pthread_mutex_lock */
    if (pthread_mutex_lock(&seat_lock) != 0) {
        fprintf(stderr, "[Agent %d] CRITICAL ERROR: Mutex lock failed\n", req->agent_id);
        pthread_exit(NULL);
    }

    /* Thao tác Check-and-Deduct nguyên tử */
    if (seats_available >= req->seats_wanted) {
        seats_available -= req->seats_wanted;
        seats_sold += req->seats_wanted;
        printf("[Agent %d] CONFIRMED: %d %s for %s.  Remaining: %d\n",
               req->agent_id, req->seats_wanted, 
               (req->seats_wanted > 1) ? "seats" : "seat", 
               req->customer, seats_available);
    } else {
        failed_bookings++;
        printf("[Agent %d] SOLD OUT:  needs %d seats, only %d left — booking failed.\n",
               req->agent_id, req->seats_wanted, seats_available);
    }

    if (pthread_mutex_unlock(&seat_lock) != 0) {
        fprintf(stderr, "[Agent %d] CRITICAL ERROR: Mutex unlock failed\n", req->agent_id);
    }
    /* --- KẾT THÚC CRITICAL SECTION --- */

    pthread_exit(NULL);
}

int main(void) {
    BookingRequest requests[NUM_AGENTS] = {
        {1, "Nguyen Van An",  2},
        {2, "Tran Thi Bich",  1},
        {3, "Le Van Cuong",   3},
        {4, "Pham Thi Dung",  1},
        {5, "Hoang Van Em",   2}
    };

    pthread_t threads[NUM_AGENTS];
    int threads_spawned[NUM_AGENTS] = {0}; /* Mảng đánh dấu luồng khởi tạo thành công để tránh lỗi khi join */
    int rc;

    printf("==============================================\n");
    printf("   TICKET BOOKING SYSTEM (5 agents, 10 seats)\n");
    printf("==============================================\n");

    /* Kiểm tra return value khi khởi tạo Mutex */
    rc = pthread_mutex_init(&seat_lock, NULL);
    if (rc != 0) {
        fprintf(stderr, "CRITICAL ERROR: Mutex initialization failed (code: %d)\n", rc);
        return EXIT_FAILURE;
    }

    /* Khởi tạo các Agent Threads */
    for (int i = 0; i < NUM_AGENTS; i++) {
        rc = pthread_create(&threads[i], NULL, agent_worker, &requests[i]);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Failed to create thread for Agent %d (code: %d)\n", requests[i].agent_id, rc);
            
            /* SỬA LỖI LOGIC: Khóa mutex để tăng failed_bookings một cách an toàn, không double-count ở luồng con */
            if (pthread_mutex_lock(&seat_lock) == 0) {
                failed_bookings++;
                pthread_mutex_unlock(&seat_lock);
            }
            threads_spawned[i] = 0; /* Đánh dấu luồng này lỗi, không tạo được */
        } else {
            threads_spawned[i] = 1; /* Đánh dấu luồng tạo thành công */
        }
    }

    /* Thu hồi và đồng bộ trạng thái các luồng */
    for (int i = 0; i < NUM_AGENTS; i++) {
        if (threads_spawned[i] == 1) {
            rc = pthread_join(threads[i], NULL);
            if (rc != 0) {
                fprintf(stderr, "ERROR: pthread_join failed for thread %d (code: %d)\n", i, rc);
            }
        }
    }

    printf("\n================ SUMMARY ================\n");
    printf("  Total seats     : %d\n", MAX_SEATS);
    printf("  Seats sold      : %d\n", seats_sold);
    printf("  Seats remaining : %d\n", seats_available);
    printf("  Failed bookings : %d\n", failed_bookings);
    printf("=========================================\n");

    /* Kiểm tra return value khi hủy Mutex */
    rc = pthread_mutex_destroy(&seat_lock);
    if (rc != 0) {
        fprintf(stderr, "ERROR: Mutex destruction failed (code: %d)\n", rc);
    }

    return EXIT_SUCCESS;
}

/* ============================================================================
 * COMMENT BLOCK GIẢI THÍCH (YÊU CẦU ĐẶC TẢ)
 * * Tại sao thao tác "Check" (Kiểm tra) và "Deduct" (Trừ số lượng) PHẢI nằm trong 
 * cùng một khối lock/unlock (Single Critical Section)?
 * * Nếu tách thành 2 khối khóa riêng biệt: Lock(); Check(); Unlock(); -> Lock(); Deduct(); Unlock();
 * Nguy cơ xảy ra là: Luồng A kiểm tra thấy còn 2 ghế (thỏa mãn), liền nhả Lock. Ngay lập tức Luồng B 
 * chiếm Lock, cũng kiểm tra thấy còn 2 ghế (thỏa mãn) vì Luồng A chưa kịp trừ. Cả 2 luồng đều nhảy vào 
 * khối Deduct để trừ ghế, dẫn đến việc bán vượt mức số ghế thực tế (Overbooking). 
 * Do đó, Check-and-Deduct bắt buộc phải là một thao tác nguyên tử (Atomic operation).
 * ============================================================================ */