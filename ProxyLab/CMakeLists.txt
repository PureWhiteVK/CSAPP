cmake_minimum_required(VERSION 3.20.0)

project(ProxyLab)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(add-antlr4)
include(FetchContent)

find_package(Threads REQUIRED)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(spdlog)

add_antlr4_jar(4.13.0)
add_antlr4_cpp(4.13.0 STATIC)

add_antlr4_target(
  url_parser 
  INPUT_FILES url.g4 
  NAMESPACE parser
  GEN_VISITOR
)

# url_parser library
add_library(url_parser STATIC 
  ${antlr4_url_parser_SOURCES})
target_include_directories(url_parser PUBLIC ${antlr4_url_parser_INCLUDE_DIR})
target_link_libraries(url_parser PUBLIC antlr4::antlr4_static)
set_property(TARGET url_parser PROPERTY CXX_STANDARD 17)
set_property(TARGET url_parser PROPERTY CXX_STANDARD_REQUIRED TRUE)
add_library(parser::url_parser ALIAS url_parser)

# csapp library
add_library(csapp STATIC csapp.c)
target_include_directories(csapp PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(csapp PUBLIC Threads::Threads)
add_library(csapp::csapp ALIAS csapp)

# proxy executable
add_executable(proxy proxy.cpp)
target_sources(proxy PRIVATE 
  request_parser.cpp 
  response_parser.cpp
  string_utils.cpp
  thread_pool.cpp
  url_visitor.cpp
  cache.cpp
)
target_link_libraries(proxy PRIVATE csapp::csapp spdlog::spdlog parser::url_parser)
set_property(TARGET proxy PROPERTY CXX_STANDARD 17)
set_property(TARGET proxy PROPERTY CXX_STANDARD_REQUIRED TRUE)

# copy proxy executable to ${PROJECT_SOURCE_DIR}
add_custom_command(
  TARGET proxy POST_BUILD 
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:proxy>" "${PROJECT_SOURCE_DIR}/$<TARGET_FILE_NAME:proxy>"
)