# minimum supported cmake version
cmake_minimum_required(VERSION 2.6)

# identify the project and programming language
project(engine C)

# compiler flags passed as arguments 
add_definitions(-Wall -std=c99)

# specify the TARGET executable
add_executable(engine auto_assign.c
										  auto_comps.c
										  base.c
										  branch.c
										  echo.c
										  error_handlers.c
										  filter.c
										  flowy.c
										  ftreader.c
										  grouper.c
										  groupfilter.c
										  merger.c
										  ungrouper.c
										  utils.c)

# specify linked libraries
target_link_libraries(engine m pthread ft json z)