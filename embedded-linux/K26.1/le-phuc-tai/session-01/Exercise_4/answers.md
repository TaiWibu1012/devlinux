# Answers for Exercise 4

## Step 1 — Create a regular variable
- **Kết quả:** Lệnh `echo $MY_NAME` hiển thị kết quả là `Quan`.
- **Giải thích:** Biến được khởi tạo thành công và có giá trị trong phiên làm việc (shell) hiện tại.

## Step 2 — Open a child shell and check
- **Kết quả:** Lệnh `echo $MY_NAME` bên trong child shell không hiển thị gì cả (trống).
- **Giải thích:** Vì `MY_NAME` ở Bước 1 chỉ là một biến cục bộ (local variable) của shell cha. Theo mặc định, shell con (child shell) sẽ không được thừa hưởng các biến cục bộ từ shell cha truyền sang.

## Step 3 — Export the variable
- **Kết quả:** Lệnh `echo $MY_NAME` bên trong child shell lúc này đã hiển thị giá trị `Quan`.
- **Giải thích:** Lệnh `export` đã biến cục bộ `MY_NAME` thành một biến môi trường (environment variable). Khi một biến đã trở thành biến môi trường, mọi tiến trình con hoặc shell con sinh ra từ shell cha đó đều sẽ được sao chép và thừa hưởng biến này.

## Step 4 — Modify variable inside child shell
- **Kết quả:**
  - Ở trong child shell: `echo $MY_NAME` hiển thị `Alice`.
  - Sau khi `exit` về parent shell: `echo $MY_NAME` hiển thị `Quan`.
- **Giải thích:** Do tính chất cô lập vùng nhớ giữa các tiến trình (Process Isolation). Khi shell con thay đổi giá trị của biến môi trường, nó chỉ thay đổi trên bản sao thuộc vùng nhớ của chính nó. Shell con không có quyền và không thể tác động ngược làm thay đổi giá trị biến ở vùng nhớ của shell cha   
# Trigger bot review
