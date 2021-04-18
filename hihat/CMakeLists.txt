find_package(ALSA)
add_executable(hihat.out 
main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (hihat.out ${ALSA_LIBRARIES})      
set_target_properties(hihat.out PROPERTIES CXX_STANDARD 17)


