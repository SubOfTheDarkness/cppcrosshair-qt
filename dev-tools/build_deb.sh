#!/bin/bash
set -e

if ! docker image inspect cppcrosshair-builder &> /dev/null; then
    echo "Создание локального сборочного образа для CppCrosshair через BuildKit..."
    
    cat << 'EOF' > Dockerfile.tmp
FROM ubuntu:24.04
RUN apt-get update -y && \
    apt-get install -y cmake make g++ qt6-base-dev libx11-dev libxext-dev libxpm-dev && \
    rm -rf /var/lib/apt/lists/*
EOF

    DOCKER_BUILDKIT=1 docker build --progress=plain -t cppcrosshair-builder -f Dockerfile.tmp .
    rm -f Dockerfile.tmp
    echo "Образ для сборки успешно создан и сохранен"
fi

echo "Запуск компиляции проекта CppCrosshair..."

docker run --rm \
    -v "$(pwd)":/workspace \
    cppcrosshair-builder /bin/bash -c "
        cd /workspace && \
        cmake -S . -B build-deb -DCMAKE_BUILD_TYPE=Release && \
        cd build-deb && \
        make package
    "

echo "======================================================="
echo "Сборка CppCrosshair успешно завершена"
echo "$(ls build-deb/*.deb 2>/dev/null || echo 'Пакет не найден')"
echo "======================================================="
