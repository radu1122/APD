cmake_minimum_required(VERSION 3.20)
project(tema1 C)

set(CMAKE_C_STANDARD 11)
set(GCC_COVERAGE_COMPILE_FLAGS "-lpthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
include_directories(skel)

add_executable(tema1
        skel/individual.h
        skel/sack_object.h
#        skel/genetic_algorithm.c
#        skel/genetic_algorithm.h
#        skel/tema1.c

        skel/tema1_par.c
        skel/genetic_algorithm_par.c
        skel/genetic_algorithm_par.h)
