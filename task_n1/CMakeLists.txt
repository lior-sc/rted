cmake_minimum_required(VERSION 3.12)
project(MyProject)

set(CMAKE_C_STANDARD 11)

# Add your source files here
set(SOURCES
    task1.c
)

# Create the executable
add_executable(MyExecutable ${SOURCES})