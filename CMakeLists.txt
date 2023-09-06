cmake_minimum_required(VERSION 3.10)

project(BTP LANGUAGES CXX)

# enable AVX2
add_compile_options(-maes -mavx2 -masm=intel -s)
# link libssl-dev to rapl_threaded
link_libraries(ssl crypto)
include_directories(include)

add_executable(rapl_simple src/rapl_simple.cpp)
add_executable(rapl_threaded src/rapl_threaded.cpp src/computations.cpp src/aes.cpp)
add_executable(test src/test.cpp src/computations.cpp src/aes.cpp)


