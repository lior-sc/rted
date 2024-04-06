cmake_minimum_required(VERSION 3.12)
project(PangoSTM32)

# Find Boost
find_package(Boost REQUIRED)

# Include Boost headers directory
include_directories(
    ${Boost_INCLUDE_DIRS}
    include 
    ../GPS
)

# Create a library
add_library(${PROJECT_NAME}_lib STATIC
    src/PangoSTM32.cpp
)
