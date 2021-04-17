find_package(ALSA)
add_executable(ch3.out 
AudioEvent.cpp
AudioEvent_Melody1.cpp
AudioEvent_Melody2.cpp
AudioEvent_Melody3.cpp
AudioEventMgr.cpp
AudioEvent_PlayWav.cpp
AudioEvent_PlayWavRepeat.cpp
main.cpp
oscillators.cpp
Song.cpp
utils.cpp
wavefile.cpp)
include_directories(${ALSA_INCLUDE_DIRS})                  
target_link_libraries (ch3.out ${ALSA_LIBRARIES})      
set_target_properties(ch3.out PROPERTIES CXX_STANDARD 17)

