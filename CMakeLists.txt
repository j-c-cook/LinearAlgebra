cmake_minimum_required(VERSION 3.6)
project(LinearAlgebra)

include_directories(include)

# Create a DLL with an API
add_library(LinearAlgebra SHARED
        src/blas.cpp src/lapack.cpp)

# Depends on LAPACK and BLAS
# Open blas is specified because the default (Netlib BLAS) is much slower on my intel
# Note: OpenBLAS has a copy of LAPACK in their library
set(BLA_VENDOR OpenBLAS)
set(BLAS_DIR third_party/OpenBLAS)
find_package(BLAS REQUIRED PATHS third_party/OpenBLAS)
if (BLAS_FOUND)
    target_link_libraries(LinearAlgebra ${BLAS_LIBRARIES})
else()
    # ERROR
endif()

message(STATUS ${BLAS_LIBRARIES})

# Create a test binary that depends on the DLL
add_executable(test1 test/gesv.cpp)
add_executable(test2 test/dot.cpp)
add_executable(test3 test/gemv.cpp)
add_executable(test4 test/axpy.cpp)
add_executable(test5 test/copy.cpp)
add_executable(test6 test/scal.cpp)
add_executable(test7 test/spmv.cpp)
add_executable(test8 test/cstuff.cpp)

target_link_libraries(test1 LinearAlgebra)
target_link_libraries(test2 LinearAlgebra)
target_link_libraries(test3 LinearAlgebra)
target_link_libraries(test4 LinearAlgebra)
target_link_libraries(test5 LinearAlgebra)
target_link_libraries(test6 LinearAlgebra)
target_link_libraries(test7 LinearAlgebra)
target_link_libraries(test8 LinearAlgebra)

# Turn on testing
enable_testing()

# Test the binary itself
add_test(NAME RunTest1 COMMAND ${CMAKE_BINARY_DIR}/test1)
add_test(NAME RunTest2 COMMAND ${CMAKE_BINARY_DIR}/test2)
add_test(NAME RunTest3 COMMAND ${CMAKE_BINARY_DIR}/test3)
add_test(NAME RunTest4 COMMAND ${CMAKE_BINARY_DIR}/test4)
add_test(NAME RunTest5 COMMAND ${CMAKE_BINARY_DIR}/test5)
add_test(NAME RunTest6 COMMAND ${CMAKE_BINARY_DIR}/test6)
add_test(NAME RunTest7 COMMAND ${CMAKE_BINARY_DIR}/test7)
add_test(NAME RunTest8 COMMAND ${CMAKE_BINARY_DIR}/test8)
