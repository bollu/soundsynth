find_package(ALSA)
add_executable(harp.out 
main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (harp.out ${ALSA_LIBRARIES})      
set_target_properties(harp.out PROPERTIES CXX_STANDARD 17)


