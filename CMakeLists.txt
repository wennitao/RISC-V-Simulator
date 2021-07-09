cmake_minimum_required(VERSION 3.15)
project(code)

set(CXX_FLAGS -std=c++11 -Ofast)

add_executable(code decode.hpp main.cpp main.h runcode.hpp InstructionQueue.hpp LoadStoreBuffer.hpp RegisterStatus.hpp ReorderBuffer.hpp ReservationStation.hpp)