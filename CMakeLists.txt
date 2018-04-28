cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(site-archive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=5 -Wundef -Wswitch-default -fno-omit-frame-pointer)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING -DASIO_NO_DEPRECATED)

add_executable(main main.cpp)
target_link_libraries(main PUBLIC site-archive::downloader)

find_package(Boost 1.66 REQUIRED COMPONENTS system coroutine)
find_package(OpenSSL REQUIRED)

find_package(Threads REQUIRED)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )

add_subdirectory(lib)
enable_testing()
add_subdirectory(test)

