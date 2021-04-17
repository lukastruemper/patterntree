cmake_minimum_required (VERSION 3.16)

project(Samples)

add_executable(mxv mxv.cpp)
add_executable(jacobi jacobi.cpp)
add_executable(kmeans kmeans.cpp)
add_executable(mandelbrot mandelbrot.cpp)

target_link_libraries(mxv patterntree)
target_link_libraries(jacobi patterntree)
target_link_libraries(kmeans patterntree)
target_link_libraries(mandelbrot patterntree)
