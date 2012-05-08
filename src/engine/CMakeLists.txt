# ---------------------------------------------------------------------------#
# ------------------------------- CMake -------------------------------------#
# ---------------------------------------------------------------------------#

# minimum supported cmake version
cmake_minimum_required(VERSION 2.6)

# identify the project and programming language
project(engine C)

# compiler flags passed as arguments 
add_definitions(-Wall -std=c99)

# specify the header files location
INCLUDE_DIRECTORIES(include) 

# specify the TARGET executable
file(GLOB engine_SOURCES *.c)
add_executable(engine ${engine_SOURCES})

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