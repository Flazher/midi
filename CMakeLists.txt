cmake_minimum_required(VERSION 3.8)
project(midi)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp util.cpp util.h MIDI.cpp MIDI.h)
add_executable(midi ${SOURCE_FILES})