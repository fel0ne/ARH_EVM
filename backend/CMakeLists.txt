cmake_minimum_required(VERSION 3.10)
project(AutoPartsStore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../backend/third_party)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXT_DIR}/cpp-httplib
    ${EXT_DIR}/json/include
)



add_executable(server_app 
    main.cpp 
)

find_package(Threads REQUIRED)
target_link_libraries(server_app PRIVATE Threads::Threads)

