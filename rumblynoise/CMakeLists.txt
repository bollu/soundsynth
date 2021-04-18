find_package(ALSA)
add_executable(rumblynoise.out main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (rumblynoise.out ${ALSA_LIBRARIES})      
set_target_properties(rumblynoise.out PROPERTIES CXX_STANDARD 17)


