cmake_minimum_required(VERSION 3.15)

project(Adapt-CMSA-STD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(evrp-tw-spd
    src/evrp_tw_spd_solver.cpp
    src/operator.cpp 
    src/search_framework.cpp
    src/solution.cpp
    src/util.cpp
    src/data.cpp
)
