# This file handles building each task as sub-project.
#
# Each task becomes an independent project in which you can experiment.
#
# Tasks are added as separate sub-projects so as compilation errors in
# one task don't affect another task.

# This is the main project
cmake_minimum_required(VERSION 3.0)
project(client)
set(CMAKE_CXX_STANDARD )
add_executable(client client.cpp)


# Add tasks subprojects
include(cmake/utils.cmake)
add_subprojects(${CMAKE_SOURCE_DIR})
