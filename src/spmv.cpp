//
// Created by jackcook on 6/28/21.
//

#include <LinearAlgebra/spmv.h>

namespace jcc { namespace blas {

    void spmv(int &n, double &alpha, vector<double> &A, vector<double> &x,
              double &beta, vector<double> &y, int &start, int &n_threads) {
        #pragma omp parallel for num_threads(n_threads)
        for (int i=0; i<n; i++) {
            for (int j=0; j<n; j++)  {
                y[i] = A[i + j] * x[j + start] + beta * y[i];
            }
        }
    }

}  // namespace blas
}  // namespace jcc