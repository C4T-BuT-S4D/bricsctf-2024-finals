cmake_minimum_required(VERSION 3.15)
project(simple CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(libpqxx REQUIRED)
find_package(Crow REQUIRED)

add_executable(${PROJECT_NAME} src/main.cc src/app.cc src/util.cc src/sha1.cc src/auth.cc src/note.cc)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} libpqxx::pqxx Crow::Crow)
