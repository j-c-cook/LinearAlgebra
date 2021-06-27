//
// Created by jackcook on 5/14/21.
//

#include <axpy.hh>

#include <iostream>
#include <vector>

int main(){
    // y = a*x + y

    int n = 3;  // number of elements in the vectors
    std::vector<double> x = {4, 8, 12};  // x vector
    double a = -1;  // constant to multiply x by

    int incx = 1;  // increment is 1

    std::vector<double> y = {10, 10, 10};  // y vector

    int incy = 1;  // increment is 1

    blas::axpy(n, a, &*x.begin(), incx, &*y.begin(), incy);

    for (int i = 0; i < y.size(); i++){
        std::cout << y[i] << std::endl;
    }

    return 0;
}