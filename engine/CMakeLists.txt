# ---------------------------------------------------------------------------#
# ------------------------------- CMake -------------------------------------#
# ---------------------------------------------------------------------------#

# macros
include(CheckFunctionExists)

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
                    ${CMAKE_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_BINARY_DIR}
)

# specify the TARGET executable
file(GLOB engine_SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
add_executable(engine ${CMAKE_CURRENT_BINARY_DIR}/auto-assign.c
                      ${CMAKE_CURRENT_BINARY_DIR}/auto-comps.c
                      ${engine_SOURCES}
)

# add a custom command to generate examples
file(GLOB pyFILES ${CMAKE_SOURCE_DIR}/scripts/queries/*.py)
foreach(pyFILE ${pyFILES})
  set(query "${pyFILE}_query")
  add_custom_command (
    OUTPUT ${query}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/
    COMMAND python ${pyFILE}
    COMMENT "Generating: JSON example query using ${pyFILE}"
  )
  list(APPEND queryFILES ${query})
endforeach(pyFILE)

# add a custom target for examples
add_custom_target(examples DEPENDS ${queryFILES})

# add dependency
add_dependencies(engine examples)

# find linked libraries
find_package(Threads)
find_library(MLIB m)
find_library(ZLIB z)
find_library(FTLIB ft)
find_library(JSONLIB json)
find_library(FIXBUF fixbuf)
find_package(PkgConfig)
pkg_check_modules(GLIB glib-2.0)
if (GLIB_FOUND)
  message("Found glib-2.0")
  include_directories(${GLIB_INCLUDE_DIRS})
endif (GLIB_FOUND)


# On FreeBSD libc doesn't directly provide libexecinfo, so we have to find it
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  find_library(LIBEXECINFO execinfo)
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

# specify linked libraries
target_link_libraries(engine ${MLIB}
                             ${CMAKE_THREAD_LIBS_INIT}
                             ${FTLIB}
                             ${JSONLIB}
                             ${ZLIB}
                             ${FIXBUF}
                             ${GLIB_LIBRARIES}
                             ${LIBEXECINFO})

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
SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
#SET(CPACK_STRIP_FILES "bin/MyExecutable")
#SET(CPACK_SOURCE_STRIP_FILES "")
#makeSET(CPACK_PACKAGE_EXECUTABLES "engine")
INCLUDE(CPack)

# ---------------------------------------------------------------------------#
