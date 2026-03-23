cmake_minimum_required(VERSION 3.10)
project(AutoPartsStore)

# Стандарт C++17 обязателен для современных либ и твоих алгоритмов
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Пути к сабмодулям (относительно папки backend)
set(EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../backend/third_party)

# Добавляем пути к заголовочным файлам (header-only либы)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXT_DIR}/cpp-httplib
    ${EXT_DIR}/json/include
)



# Главный исполняемый файл (Сервер)
add_executable(server_app 
    main.cpp 
    # database.h и algorithms.h подтянутся автоматически через include
)

# Если нужны дополнительные системные либы (например, для потоков в httplib)
find_package(Threads REQUIRED)
target_link_libraries(server_app PRIVATE Threads::Threads)

