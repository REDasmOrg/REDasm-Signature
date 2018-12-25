cmake_minimum_required(VERSION 2.8)

project(REDSigC)

set(CMAKE_CXX_STANDARD 11)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LC)

add_subdirectory(LibREDasm)

set(SOURCES
    main.cpp
    redsigc.cpp
    patterngenerator.cpp
    listingconsolerenderer.cpp
    psx/psyqlib.cpp)

set(HEADERS
    redsigc.h
    patterngenerator.h
    listingconsolerenderer.h
    psx/psyqlib.h)

file(GLOB GENERATORS
     generators/*.h
     generators/*.cpp)

add_executable(${PROJECT_NAME_LC} ${SOURCES} ${HEADERS} ${GENERATORS})
add_dependencies(${PROJECT_NAME_LC} LibREDasm)

if(WIN32)
    set(DEPENDS_INCLUDE depends depends/windows)
else()
    set(DEPENDS_INCLUDE depends)
endif()

target_include_directories(${PROJECT_NAME_LC} PUBLIC LibREDasm ${DEPENDS_INCLUDE})
target_link_libraries(${PROJECT_NAME_LC} PUBLIC LibREDasm)
