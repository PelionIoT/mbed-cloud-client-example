cmake_minimum_required(VERSION 3.10)

# set the project name
project(delta_tool_proj)

file(GLOB SOURCES *.c)

# add the executable
add_executable(delta_tool ${SOURCES})
