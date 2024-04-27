
cmake_minimum_required(VERSION 3.27)
project(mandelbrot C)
set(CMAKE_C_STANDARD 11)

add_executable(mandelbrot mandelbrot.c)
target_link_libraries(mandelbrot user32 gdi32)
target_compile_options(mandelbrot PUBLIC -Ofast)