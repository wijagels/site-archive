find_package(GTest REQUIRED)

add_executable(grab grab.cpp)
target_link_libraries(grab PRIVATE site-archive::downloader)

add_executable(jobs jobs.cpp)
target_link_libraries(jobs PRIVATE site-archive::downloader)

add_executable(echo echo.cpp)
target_link_libraries(echo PRIVATE site-archive::downloader)
