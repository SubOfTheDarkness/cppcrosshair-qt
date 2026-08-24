#!/bin/bash
set -e

if ! docker image inspect cppcrosshair-tester &> /dev/null; then
    echo "Создание локального образа для тестов..."
    
    cat <<EOF | docker build -t cppcrosshair-tester -
FROM ubuntu:24.04
RUN apt-get update -y && \
    apt-get install -y libqt6widgets6 libx11-6 libxext6 libxpm4 procps && \
    rm -rf /var/lib/apt/lists/*
EOF
    echo "Образ для тестов успешно создан"
fi

xhost +local:docker > /dev/null

DEB_FILE=$(ls build-deb/cppcrosshair-toolkit_*.deb 2>/dev/null | head -n 1)

if [ -z "$DEB_FILE" ]; then
    echo "Ошибка: .deb пакет не найден в папке build-deb/."
    exit 1
fi

echo "Найден пакет для тестирования: $DEB_FILE"

docker run -it --rm \
    --net=host \
    -e DISPLAY=$DISPLAY \
    -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
    -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:ro \
    -v "$(pwd)":/workspace \
    cppcrosshair-tester /bin/bash -c "
        dpkg -i /workspace/$DEB_FILE && \
        echo 'Пакет успешно установлен' && \
        crosshair_editor
    "

xhost -local:docker > /dev/null
