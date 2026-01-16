cmake_minimum_required(VERSION 3.10)
project(Laba_3)

set(CMAKE_CXX_STANDARD 17)

# Включаем директорию с заголовками
include_directories(include)

# Программа тестов
add_executable(tests tools/tests.cpp)

# Генератор города
add_executable(generate_city tools/generate_city.cpp)

# API для веба
add_executable(api_pathfinder tools/api_pathfinder.cpp)

