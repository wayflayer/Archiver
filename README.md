Тестовое Задание 'Цитадель'

# Кроссплатформенный архиватор

Консольный архиватор с собственным форматом архива, работающий на Windows и Linux.

## Особенности
- Создание архивов в собственном формате (только для этой программы)
- Поддержка файлов и директорий
- Прогресс-бар при работе
- Обработка прерываний (Ctrl+C)

## Установка и Запуск

### Windows 
1. Установите зависимости:
   ```cmd
   vcpkg install boost:x64-windows
2. Запуск программы
    ```cmd
   build.bat
   

### Linux
1. Установите зависимости
    ```cmd
    sudo apt-get install libboost-iostreams-dev libboost-system-dev libboost-filesystem-dev
    
2. Запуск 
    ```cmd
    chmod +x build.sh
    ./build.sh



### Использовование
1. Архивирование
    ```cmd
    myapp pack <input_path> <archive_name.myarc>
2. Разархивирование
    ```cmd
    myapp unpack <archive_name.myarc> <output_path>

### Примеры
```cmd
    myapp pack document.txt backup.myarc
    myapp pack ./photos photos_backup.myarc
    myapp unpack backup.myarc ./restored_files

Особенности реализации
Формат архива: заголовки FILE: и SIZE: + gzip-сжатие
Кроссплатформенность: Windows (статическая линковка) / Linux (динамическая)
Автоматическая обработка путей для разных ОС

Сборка
Архитектура: монолитная

Windows
Сборка через build.bat (использует Visual Studio 2022)
Готовый .exe в папке build\Release

Linux
Сборка через build.sh
Готовый бинарник в папке build# Archiver
