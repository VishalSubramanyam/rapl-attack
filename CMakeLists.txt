cmake_minimum_required(VERSION 3.10)

project(BTP LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
# enable AVX2
add_compile_options(-maes -mavx2 -masm=intel -s)
# link libssl-dev to rapl_threaded
link_libraries(ssl crypto)
include_directories(include)

add_executable(rapl_simple src/rapl_simple.cpp)
add_executable(rapl_threaded src/rapl_threaded.cpp src/computations.cpp src/aes.cpp)
add_executable(test_suite src/test_suite.cpp src/computations.cpp src/aes.cpp)
add_executable(testMSR src/msr_test.cpp)


