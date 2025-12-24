cmake_minimum_required(VERSION 3.16)
project(OOP_Lab7)

set(CMAKE_CXX_STANDARD 17)

add_executable(main
    main.cpp
    npc.cpp
    game.cpp
)

find_package(GTest REQUIRED)
add_executable(tests
    tests.cpp
    npc.cpp
    game.cpp
)
target_link_libraries(tests GTest::gtest GTest::gtest_main)
