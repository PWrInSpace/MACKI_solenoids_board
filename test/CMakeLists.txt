# This is the project CMakeLists.txt file for the test subproject
cmake_minimum_required(VERSION 3.16)

# Include the components directory of the main application:
#
set(EXTRA_COMPONENT_DIRS "../components")

# Set the components to include the tests for.
# This can be overriden from CMake cache:
# - when invoking CMake directly: cmake -D TEST_COMPONENTS="xxxxx" ..
# - when using idf.py: idf.py -T xxxxx build
#


set(TEST_FILES_DIRS
    "test"
)



set(TEST_COMPONENTS ${TEST_FILES_DIRS} CACHE STRING "List of components to test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(MACKI_solenoids_board)

# spiffs_create_partition_image(storage test_spiffs_data FLASH_IN_PROJECT)
