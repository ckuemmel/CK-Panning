cmake_minimum_required(VERSION 3.15)
project(CK_Panning VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(JUCE_DIR "/Users/carsten/Documents/GitHub")
list(APPEND CMAKE_PREFIX_PATH "${JUCE_DIR}")
find_package(JUCE CONFIG REQUIRED)

add_subdirectory(Source)
