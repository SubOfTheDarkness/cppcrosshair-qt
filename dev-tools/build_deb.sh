#!/bin/bash
set -e

if ! docker image inspect cppcrosshair-builder &> /dev/null; then
    echo "Создание локального сборочного образа..."
    
    cat <<EOF | docker build -t cppcrosshair-builder -
FROM ubuntu:24.04
RUN apt-get update -y && \
    apt-get install -y cmake make g++ qt6-base-dev libx11-dev libxext-dev libxpm-dev && \
    rm -rf /var/lib/apt/lists/*
EOF
    echo "Образ успешно создан и сохранен в системе"
fi

echo "Запуск компиляции проекта..."

docker run --rm \
    -v "$(pwd)":/workspace \
    cppcrosshair-builder /bin/bash -c "
        cd /workspace && \
        cmake -S . -B build-deb -DCMAKE_BUILD_TYPE=Release && \
        cd build-deb && \
        make package
    "

echo "======================================================="
echo "Сборка успешно завершена!"
echo "$(ls build-deb/cppcrosshair-toolkit_*.deb)"
echo "======================================================="
