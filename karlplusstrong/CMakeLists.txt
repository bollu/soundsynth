find_package(ALSA)
add_executable(karlplusstrong.out 
main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (karlplusstrong.out ${ALSA_LIBRARIES})      
set_target_properties(karlplusstrong.out PROPERTIES CXX_STANDARD 17)


