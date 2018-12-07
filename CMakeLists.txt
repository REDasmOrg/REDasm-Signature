cmake_minimum_required(VERSION 2.8)

project(REDSigC)

set(CMAKE_CXX_STANDARD 11)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)

add_subdirectory(LibREDasm)

set(SOURCES
    main.cpp
    parsers/psyqlib.cpp)

set(HEADERS
    parsers/psyqlib.h)

add_executable(${PROJECT_NAME_LC} ${SOURCES} ${HEADERS})
add_dependencies(${PROJECT_NAME_LC} LibREDasm)

target_include_directories(${PROJECT_NAME_LC} PUBLIC LibREDasm)
target_link_libraries(${PROJECT_NAME_LC} PUBLIC LibREDasm)
