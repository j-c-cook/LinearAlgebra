//
// Created by jackcook on 5/21/21.
//

#include <cblas.h>

#include <iostream>

int main() {
    openblas_set_num_threads(4);
    goto_set_num_threads(4);

    int nThreads = openblas_get_num_threads();

    std::cout << "Number of threads allocated to OpenBLAS: ";
    std::cout << nThreads << std::endl;

    return 0;
}