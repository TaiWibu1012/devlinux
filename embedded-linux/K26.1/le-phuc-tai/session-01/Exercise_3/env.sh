#!/bin/bash

echo "=== 1. Giá trị của các biến môi trường ==="
echo "PATH: $PATH"
echo "HOME: $HOME"
echo "USER: $USER"
echo "SHELL: $SHELL"
echo ""

echo "=== 2. Số lượng thư mục trong PATH ==="
echo -n "Số lượng: "
echo "$PATH" | tr ':' '\n' | wc -l
echo ""

echo "=== 3. Tổng số lượng biến môi trường hệ thống ==="
echo -n "Tổng số biến: "
printenv | wc -l  