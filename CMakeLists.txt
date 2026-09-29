cmake_minimum_required(VERSION 3.21)
project(Neverlose)

include($ENV{GEODE_SDK}/cmake/Geode.cmake)

add_geode_mod(
    Neverlose
    SOURCES
        src/main.cpp
)
