cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(site-archive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=5 -Wundef -Wswitch-default -fno-omit-frame-pointer)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING -DBOOST_ASIO_NO_DEPRECATED -DBOOST_ASIO_HAS_STD_STRING_VIEW)

find_package(PkgConfig REQUIRED)
find_package(Boost 1.66 REQUIRED COMPONENTS system coroutine)
find_package(OpenSSL REQUIRED)
pkg_check_modules(libcrypto++ REQUIRED IMPORTED_TARGET libcrypto++)

find_package(Threads REQUIRED)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )

add_subdirectory(lib)
enable_testing()
add_subdirectory(test)

