#pragma once
#if __has_include(<filesystem>)
#include <filesystem>
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace std::filesystem {
using namespace ::std::experimental::filesystem;
}
#else
#pragma error("No std::filesystem support available!")
#endif
