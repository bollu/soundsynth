find_package(ALSA)
add_executable(flanger.out main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (flanger.out ${ALSA_LIBRARIES})      
set_target_properties(flanger.out PROPERTIES CXX_STANDARD 17)


