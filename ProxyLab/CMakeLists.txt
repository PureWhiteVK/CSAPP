cmake_minimum_required(VERSION 3.25.0)
project(ProxyLab)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(spdlog)

# csapp library
add_library(csapp csapp.c)
target_include_directories(csapp PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(csapp PUBLIC Threads::Threads)
add_library(csapp::csapp ALIAS csapp)

# proxy executable
add_executable(proxy proxy.cpp)
target_link_libraries(proxy PRIVATE csapp::csapp spdlog::spdlog)
set_property(TARGET proxy PROPERTY CXX_STANDARD 17)
set_property(TARGET proxy PROPERTY CXX_STANDARD_REQUIRED TRUE)

# copy proxy executable to ${PROJECT_SOURCE_DIR}
add_custom_command(
  TARGET proxy POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:proxy>" "${PROJECT_SOURCE_DIR}/$<TARGET_FILE_NAME:proxy>"
)