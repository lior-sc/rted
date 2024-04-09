cmake_minimum_required(VERSION 3.12)
project(PangoServer)

# Include Boost headers directory
include_directories(
    include 
    ../GPS
)

# Create a library
add_library(${PROJECT_NAME}_lib STATIC
    src/PangoServer.cpp
)

# Create an executable
add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib)


