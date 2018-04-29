add_library(downloader worker.cpp archiver.cpp http_client.cpp job_manager.cpp log.cpp)
target_link_libraries(downloader PUBLIC site-archive::net)
target_link_libraries(downloader PUBLIC Boost::boost Boost::system Boost::coroutine Threads::Threads PkgConfig::libcrypto++ stdc++fs)
target_include_directories(downloader PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
add_library(site-archive::downloader ALIAS downloader)

add_subdirectory(net)

# if(NOT CLANG_TIDY_EXE)
#   message(STATUS "clang-tidy not found.")
# else()
#   message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
#   set_target_properties(
#       downloader
#     PROPERTIES
#     CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
#   )
# endif()
