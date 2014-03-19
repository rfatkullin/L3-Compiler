project(L3-Compiler)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER g++)

include_directories(${CMAKE_SOURCE_DIR}/build)
include_directories(${CMAKE_SOURCE_DIR}/backend)
include_directories(${CMAKE_SOURCE_DIR}/frontend)

file(GLOB FRONTEND_SRC_LIST ${PROJECT_SOURCE_DIR}/frontend/*.cpp ${PROJECT_SOURCE_DIR}/frontend/*.c ${PROJECT_SOURCE_DIR}/frontend/*.h ${PROJECT_SOURCE_DIR}/frontend/*.y ${PROJECT_SOURCE_DIR}/frontend/*.l)
file(GLOB BACKEND_SRC_LIST ${PROJECT_SOURCE_DIR}/backend/*.cpp ${PROJECT_SOURCE_DIR}/backend/*.c ${PROJECT_SOURCE_DIR}/backend/*.h)
file(GLOB BUILD_SRC_LIST ${PROJECT_SOURCE_DIR}/build/*.cpp ${PROJECT_SOURCE_DIR}/build/*.c ${PROJECT_SOURCE_DIR}/build/*.h)

SET(SRC_LIST ${FRONTEND_SRC_LIST} ${BACKEND_SRC_LIST} ${BUILD_SRC_LIST})
MESSAGE( STATUS "BE: " ${SRC_LIST} )
MESSAGE( STATUS "BE: " ${CMAKE_CXX_COMPILER} )

set_source_files_properties(parser.y PROPERTIES LANGUAGE CXX )

add_executable(${PROJECT_NAME} ${SRC_LIST})

