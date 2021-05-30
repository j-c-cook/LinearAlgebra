// Copyright (c) 2017-2021, University of Tennessee. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause
// This program is free software: you can redistribute it and/or modify it under
// the terms of the BSD 3-Clause license. See the accompanying LICENSE file.

#include "test.hh"
#include "cblas_wrappers.hh"
#include "lapack_wrappers.hh"
#include "blas/flops.hh"
#include "print_matrix.hh"

// -----------------------------------------------------------------------------
template< typename TX, typename TY >
void test_swap_device_work( Params& params, bool run )
{
    using namespace testsweeper;
    using namespace blas;
    using scalar_t = blas::scalar_type< TX, TY >;
    using real_t = blas::real_type< scalar_t >;
    typedef long long lld;

    // get & mark input values
    int64_t n       = params.dim.n();
    int64_t incx    = params.incx();
    int64_t incy    = params.incy();
    int64_t device  = params.device();
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

    if (blas::get_device_count() == 0) {
        printf("skipping: no GPU devices or no GPU support\n" );
        return;
    }

    // setup
    size_t size_x = (n - 1) * std::abs(incx) + 1;
    size_t size_y = (n - 1) * std::abs(incy) + 1;
    TX* x    = new TX[ size_x ];
    TX* xref = new TX[ size_x ];
    TY* y    = new TY[ size_y ];
    TY* yref = new TY[ size_y ];

    // device specifics
    blas::Queue queue(device,0);
    TX* dx;
    TY* dy;

    dx = blas::device_malloc<TX>(size_x);
    dy = blas::device_malloc<TY>(size_y);

    int64_t idist = 1;
    int iseed[4] = { 0, 0, 0, 1 };
    lapack_larnv( idist, iseed, size_x, x );
    lapack_larnv( idist, iseed, size_y, y );
    cblas_copy( n, x, incx, xref, incx );
    cblas_copy( n, y, incy, yref, incy );

    // todo: should we have different incdx and incdy
    blas::device_setvector(n, x, std::abs(incx), dx, std::abs(incx), queue);
    blas::device_setvector(n, y, std::abs(incy), dy, std::abs(incy), queue);
    queue.sync();

    // test error exits
    assert_throw( blas::swap( -1, dx, incx, dy, incy ), blas::Error );
    assert_throw( blas::swap(  n, dx,    0, dy, incy ), blas::Error );
    assert_throw( blas::swap(  n, dx, incx, dy,    0 ), blas::Error );

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
    blas::swap( n, dx, incx, dy, incy, queue );
    queue.sync();
    time = get_wtime() - time;

    double gflop = Gflop < scalar_t >::swap( n );
    double gbyte = Gbyte < scalar_t >::swap( n );
    params.time()   = time * 1000;  // msec
    params.gflops() = gflop / time;
    params.gbytes() = gbyte / time;

    // todo: should we have different incdx and incdy
    blas::device_getvector(n, dx, std::abs(incx), x, std::abs(incx), queue);
    blas::device_getvector(n, dy, std::abs(incy), y, std::abs(incy), queue);
    queue.sync();

    if (verbose >= 2) {
        printf( "x2   = " ); print_vector( n, x, incx );
        printf( "y2   = " ); print_vector( n, y, incy );
    }

    if (params.check() == 'y') {
        // run reference
        testsweeper::flush_cache( params.cache() );
        time = get_wtime();
        cblas_swap( n, xref, incx, yref, incy );
        time = get_wtime() - time;
        if (verbose >= 2) {
            printf( "xref = " ); print_vector( n, xref, incx );
            printf( "yref = " ); print_vector( n, yref, incy );
        }

        params.ref_time()   = time * 1000;  // msec
        params.ref_gflops() = gflop / time;
        params.ref_gbytes() = gbyte / time;

        // error = ||xref - x|| + ||yref - y||
        cblas_axpy( n, -1.0, x, incx, xref, incx );
        cblas_axpy( n, -1.0, y, incy, yref, incy );
        real_t error = cblas_nrm2( n, xref, std::abs(incx) )
                     + cblas_nrm2( n, yref, std::abs(incy) );
        params.error() = error;

        // swap must be exact!
        params.okay() = (error == 0);
    }

    delete[] x;
    delete[] y;
    delete[] xref;
    delete[] yref;

    blas::device_free( dx );
    blas::device_free( dy );
}

// -----------------------------------------------------------------------------
void test_swap_device( Params& params, bool run )
{
    switch (params.datatype()) {
        case testsweeper::DataType::Single:
            test_swap_device_work< float, float >( params, run );
            break;

        case testsweeper::DataType::Double:
            test_swap_device_work< double, double >( params, run );
            break;

        case testsweeper::DataType::SingleComplex:
            test_swap_device_work< std::complex<float>, std::complex<float> >
                ( params, run );
            break;

        case testsweeper::DataType::DoubleComplex:
            test_swap_device_work< std::complex<double>, std::complex<double> >
                ( params, run );
            break;

        default:
            throw std::exception();
            break;
    }
}
