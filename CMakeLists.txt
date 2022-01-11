cmake_minimum_required(VERSION 3.20)
project(lab43)

set(CMAKE_CXX_STANDARD 11)

add_executable(lab43 main.cpp sqlite_manager.h sqlite_manager.cpp)
find_package(Crow)
target_link_libraries(lab43 LINK_PUBLIC sqlite3)
target_link_libraries(lab43 PUBLIC Crow::Crow)
