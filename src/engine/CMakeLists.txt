# ---------------------------------------------------------------------------#
# ------------------------------- CMake -------------------------------------#
# ---------------------------------------------------------------------------#

# minimum supported cmake version
cmake_minimum_required(VERSION 2.6)

# identify the project and programming language
project(engine C)

# compiler flags passed as arguments
add_definitions(-Wall -Werror -std=c99)

# set output directories for libraries and executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# add a custom command to prepare auto-generated sources
add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/auto-assign.h
         ${CMAKE_CURRENT_BINARY_DIR}/auto-assign.c
         ${CMAKE_CURRENT_BINARY_DIR}/auto-comps.h
         ${CMAKE_CURRENT_BINARY_DIR}/auto-comps.c
  COMMAND python ${CMAKE_SOURCE_DIR}/scripts/generate-functions.py
  COMMENT "Generating: auto-comps{h,c} and auto-assign.{h,c}"
)

# specify the header files location
include_directories(${CMAKE_PREFIX_PATH}/include
                    ${CMAKE_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_BINARY_DIR}
)

# specify the TARGET executable
file(GLOB engine_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
add_executable(engine ${CMAKE_CURRENT_BINARY_DIR}/auto-assign.c
                      ${CMAKE_CURRENT_BINARY_DIR}/auto-comps.c
                      ${engine_SOURCES}
)

# find linked libraries
find_package(Threads)
find_library(MLIB m)
find_library(ZLIB z)
find_library(FTLIB ft)
find_library(JSONLIB json)

# specify linked libraries
target_link_libraries(engine ${MLIB}
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${FTLIB}
                             ${JSONLIB}
                             ${ZLIB})

# ---------------------------------------------------------------------------#






# ---------------------------------------------------------------------------#
# ------------------------------- CPack -------------------------------------#
# ---------------------------------------------------------------------------#

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C Prototype Implementation of NFQL")
SET(CPACK_PACKAGE_VENDOR "Vaibhav Bajpai")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
#SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#SET(CPACK_STRIP_FILES "bin/MyExecutable")
#SET(CPACK_SOURCE_STRIP_FILES "")
#makeSET(CPACK_PACKAGE_EXECUTABLES "engine")
INCLUDE(CPack)

# ---------------------------------------------------------------------------#
