find_package(ALSA)
add_executable(whitenoise.out main.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (whitenoise.out ${ALSA_LIBRARIES})      
set_target_properties(whitenoise.out PROPERTIES CXX_STANDARD 17)


