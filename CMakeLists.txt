cmake_minimum_required(VERSION 3.10)
project(FileArchiver)

# Автоматическое определение ОС
if(WIN32)
    # Настройки для Windows
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    add_definitions(-DNOMINMAX)
else()
    # Настройки для Linux
    set(Boost_USE_STATIC_LIBS OFF)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# Поиск Boost
find_package(Boost 1.70 REQUIRED COMPONENTS iostreams system filesystem)

# Создание исполняемого файла
add_executable(myapp
    main.cpp
    src/Archiver.cpp
)

# Подключение Boost
target_link_libraries(myapp PRIVATE 
    Boost::iostreams 
    Boost::system 
    Boost::filesystem
)

# Дополнительные системные библиотеки
if(WIN32)
    target_link_libraries(myapp PRIVATE kernel32.lib)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(myapp PRIVATE Threads::Threads)
endif()