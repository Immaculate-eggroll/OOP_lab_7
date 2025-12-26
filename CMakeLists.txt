cmake_minimum_required(VERSION 3.16)
project(OOP_Lab7)

# Явно указываем компилятор
set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-15")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Для многопоточности
find_package(Threads REQUIRED)

# Основная программа
add_executable(lab7_main
    main.cpp
    npc.cpp
    game.cpp
)
target_link_libraries(lab7_main PRIVATE Threads::Threads)

# GoogleTest (если нужны тесты)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(lab7_tests
    tests.cpp
    npc.cpp
    game.cpp
)
target_link_libraries(lab7_tests PRIVATE gtest_main Threads::Threads)

include(GoogleTest)
gtest_discover_tests(lab7_tests)
