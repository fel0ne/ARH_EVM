cmake_minimum_required(VERSION 3.10)
project(AutoPartsStore)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)
    set(OPENSSL_INCLUDE_DIR /opt/homebrew/opt/openssl@3/include)
    set(OPENSSL_LIBRARIES /opt/homebrew/opt/openssl@3/lib)
endif()

set(EXT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../backend/third_party)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EXT_DIR}/cpp-httplib
    ${EXT_DIR}/json/include
)

if(APPLE)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

add_executable(server_app
    main.cpp
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_compile_definitions(server_app PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

if(APPLE)
    target_link_libraries(server_app PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        "-framework Security"
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(server_app PRIVATE
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
    )
endif()