// Copyright (c) 2017-2021, University of Tennessee. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#include "test.hh"
#include "cblas_wrappers.hh"
#include "lapack_wrappers.hh"
#include "blas/flops.hh"
#include "print_matrix.hh"
#include "check_gemm.hh"

// -----------------------------------------------------------------------------
// TX is data [x, y]
// TS is for sine, which can be real (zdrot) or complex (zrot)
// cosine is always real
template< typename TX, typename TS >
void test_rot_work( Params& params, bool run )
{
    using namespace testsweeper;
    using namespace blas;
    typedef real_type<TX> real_t;
    typedef long long lld;

    // get & mark input values
    int64_t n       = params.dim.n();
    int64_t incx    = params.incx();
    int64_t incy    = params.incy();
    int64_t verbose = params.verbose();

    // mark non-standard output values
    params.gflops();
    params.gbytes();
    params.ref_time();
    params.ref_gflops();
    params.ref_gbytes();

    // adjust header to msec
    params.time.name( "BLAS++\ntime (ms)" );
    params.ref_time.name( "Ref.\ntime (ms)" );

    if (! run)
        return;

    // setup
    size_t size_x = (n - 1) * std::abs(incx) + 1;
    size_t size_y = (n - 1) * std::abs(incy) + 1;
    TX* x    = new TX[ size_x ];
    TX* xref = new TX[ size_x ];
    TX* y    = new TX[ size_y ];
    TX* yref = new TX[ size_y ];
    TX s = rand() / double(RAND_MAX);    // todo: imag
    real_t c = sqrt( 1 - real(s*conj(s)) );  // real

    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, size_x, x );
    lapack_larnv( idist, iseed, size_y, y );
    cblas_copy( n, x, incx, xref, incx );
    cblas_copy( n, y, incy, yref, incy );

    // norms for error check
    real_t Xnorm = cblas_nrm2( n, x, std::abs(incx) );
    real_t Ynorm = cblas_nrm2( n, y, std::abs(incy) );
    real_t Anorm = sqrt( Xnorm*Xnorm + Ynorm*Ynorm ); // || [x y] ||_F

    // test error exits
    assert_throw( blas::rot( -1, x, incx, y, incy, c, s ), blas::Error );
    assert_throw( blas::rot(  n, x,    0, y, incy, c, s ), blas::Error );
    assert_throw( blas::rot(  n, x, incx, y,    0, c, s ), blas::Error );

    if (verbose >= 1) {
        printf( "\n"
                "x n=%5lld, inc=%5lld, size=%10lld\n"
                "y n=%5lld, inc=%5lld, size=%10lld\n",
                (lld) n, (lld) incx, (lld) size_x,
                (lld) n, (lld) incy, (lld) size_y );
    }
    if (verbose >= 2) {
        printf( "x    = " ); print_vector( n, x, incx );
        printf( "y    = " ); print_vector( n, y, incy );
    }

    // run test
    testsweeper::flush_cache( params.cache() );
    double time = get_wtime();
    blas::rot( n, x, incx, y, incy, c, s );
    time = get_wtime() - time;

    double gflop = Gflop < TX >::dot( n );
    double gbyte = Gbyte < TX >::dot( n );
    params.time()   = time * 1000;  // msec
    params.gflops() = gflop / time;
    params.gbytes() = gbyte / time;

    if (verbose >= 1) {
        printf( "x2   = " ); print_vector( n, x, incx );
        printf( "y2   = " ); print_vector( n, y, incy );
    }

    if (params.ref() == 'y' || params.check() == 'y') {
        // run reference
        testsweeper::flush_cache( params.cache() );
        time = get_wtime();
        cblas_rot( n, xref, incx, yref, incy, c, s );
        time = get_wtime() - time;

        params.ref_time()   = time * 1000;  // msec
        params.ref_gflops() = gflop / time;
        params.ref_gbytes() = gbyte / time;

        if (verbose >= 1) {
            printf( "xref = " ); print_vector( n, xref, incx );
            printf( "yref = " ); print_vector( n, yref, incy );
        }

        // check error compared to reference
        // C = [x y] * R for n x 2 matrix C and 2 x 2 rotation R
        // alpha=1, beta=0, C0norm=0
        TX* C    = new TX[ 2*n ];
        TX* Cref = new TX[ 2*n ];
        blas::copy( n, x,    incx, &C[0],    1 );
        blas::copy( n, y,    incy, &C[n],    1 );
        blas::copy( n, xref, incx, &Cref[0], 1 );
        blas::copy( n, yref, incy, &Cref[n], 1 );
        real_t Rnorm = sqrt(2);  // ||R||_F
        real_t error;
        bool okay;
        check_gemm( n, 2, 2, TX(1), TX(0), Anorm, Rnorm, real_t(0),
                    Cref, n, C, n, verbose, &error, &okay );
        params.error() = error;
        params.okay() = okay;

        delete[] C;
        delete[] Cref;
    }

    delete[] x;
    delete[] y;
    delete[] xref;
    delete[] yref;
}

// -----------------------------------------------------------------------------
void test_rot( Params& params, bool run )
{
    switch (params.datatype()) {
        case testsweeper::DataType::Single:
            test_rot_work< float, float >( params, run );
            break;

        case testsweeper::DataType::Double:
            test_rot_work< double, double >( params, run );
            break;

        // todo: real sine
        // todo: complex sine

        default:
            throw std::exception();
            break;
    }
}
