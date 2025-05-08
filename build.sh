#!/bin/bash

rm -rf build

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Сборка завершена. Запуск: ./build/myapp"
    ./build/myapp
else
    echo "Ошибка сборки"
    exit 1
fi