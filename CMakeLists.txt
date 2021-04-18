cmake_minimum_required(VERSION 3.4.3)
project(kavariance C CXX)

# Colors when compiling with clang
# set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always -fno-rtti -g -Wall -Werror")
set(CMAKE_CXX_FLAGS "-fdiagnostics-color=always -g -Wall")

include_directories(include ${LLVM_INCLUDE_DIRS} include/tinyfort)

add_executable(ch1.out ch1.cpp)
set_target_properties(ch1.out PROPERTIES CXX_STANDARD 17)

add_executable(ch2.out ch2.cpp)
set_target_properties(ch2.out PROPERTIES CXX_STANDARD 17)

add_subdirectory(karlplusstrong)
add_subdirectory(whitenoise)
add_subdirectory(rumblynoise)

# add_library(fort src/libfort.c)
# target_link_libraries(tinyfort ${LLVM_LIBS}  -lstdc++ ${Z3_LIBS})

