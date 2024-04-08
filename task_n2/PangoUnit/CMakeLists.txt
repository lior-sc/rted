cmake_minimum_required(VERSION 3.12)
project(PangoUNIT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find Boost
find_package(Boost REQUIRED)

# Include Boost headers directory
include_directories(
    ${Boost_INCLUDE_DIRS}
    include 
    ../GPS
)

# Create a library

add_library(${PROJECT_NAME}_lib 
    src/PangoSTM32.cpp
    src/PangoBBG.cpp
)

add_executable(PangoBBG bbg_main.cpp)
add_executable(PangoSTM32 stm32_main.cpp)

target_link_libraries(PangoSTM32 ${PROJECT_NAME}_lib)
target_link_libraries(PangoBBG ${PROJECT_NAME}_lib)