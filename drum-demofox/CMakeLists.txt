find_package(ALSA)
add_executable(drum.out main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (drum.out ${ALSA_LIBRARIES})      
set_target_properties(drum.out PROPERTIES CXX_STANDARD 17)


