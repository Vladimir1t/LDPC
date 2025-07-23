cmake_minimum_required(VERSION 3.15)

project(RS_LDPC_Algorithm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ReedSolomon/ReedSolomon/ReedSolomon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LDPC/LdpcC/LdpcCode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LDPC/LdpcC/Constellation.cpp
)

add_executable(RS_LDPC_Algorithm ${SOURCES})

target_include_directories(RS_LDPC_Algorithm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/ReedSolomon/ReedSolomon
    ${CMAKE_CURRENT_SOURCE_DIR}/LDPC/LdpcC
    # ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# find_package(aff3ct REQUIRED)
# target_link_libraries(RS_LDPC_Algorithm PRIVATE aff3ct::aff3ct)