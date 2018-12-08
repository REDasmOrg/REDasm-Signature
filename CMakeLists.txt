cmake_minimum_required(VERSION 2.8)

project(REDSigC)

set(CMAKE_CXX_STANDARD 11)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)

add_subdirectory(LibREDasm)

set(SOURCES
    main.cpp
    redsigc.cpp
    patterngenerator.cpp
    psx/psyqlib_generator.cpp
    psx/psyqlib.cpp)

set(HEADERS
    redsigc.h
    patterngenerator.h
    psx/psyqlib_generator.h
    psx/psyqlib.h)

add_executable(${PROJECT_NAME_LC} ${SOURCES} ${HEADERS})
add_dependencies(${PROJECT_NAME_LC} LibREDasm)

if(WIN32)
    target_include_directories(${PROJECT_NAME_LC} PUBLIC dirent/include)
endif()

target_include_directories(${PROJECT_NAME_LC} PUBLIC LibREDasm)
target_link_libraries(${PROJECT_NAME_LC} PUBLIC LibREDasm)
