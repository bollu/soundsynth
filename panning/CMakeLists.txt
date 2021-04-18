find_package(ALSA)
add_executable(panning.out main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (panning.out ${ALSA_LIBRARIES})      
set_target_properties(panning.out PROPERTIES CXX_STANDARD 17)


