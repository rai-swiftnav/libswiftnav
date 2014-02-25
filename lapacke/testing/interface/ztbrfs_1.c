/*****************************************************************************
  Copyright (c) 2010, Intel Corp.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/
/*  Contents: test routine for C interface to LAPACK
*   Author: Intel Corporation
*   Created in March, 2010
*
* Purpose
*
* ztbrfs_1 is the test program for the C interface to LAPACK
* routine ztbrfs
* The program doesn't require an input, the input data is hardcoded in the
* test program.
* The program tests the C interface in the four combinations:
*   1) column-major layout, middle-level interface
*   2) column-major layout, high-level interface
*   3) row-major layout, middle-level interface
*   4) row-major layout, high-level interface
* The output of the C interface function is compared to those obtained from
* the corresponiding LAPACK routine with the same input data, and the
* comparison diagnostics is then printed on the standard output having PASSED
* keyword if the test is passed, and FAILED keyword if the test isn't passed.
*****************************************************************************/
#include <stdio.h>
#include "lapacke.h"
#include "lapacke_utils.h"
#include "test_utils.h"

static void init_scalars_ztbrfs( char *uplo, char *trans, char *diag,
                                 lapack_int *n, lapack_int *kd,
                                 lapack_int *nrhs, lapack_int *ldab,
                                 lapack_int *ldb, lapack_int *ldx );
static void init_ab( lapack_int size, lapack_complex_double *ab );
static void init_b( lapack_int size, lapack_complex_double *b );
static void init_x( lapack_int size, lapack_complex_double *x );
static void init_ferr( lapack_int size, double *ferr );
static void init_berr( lapack_int size, double *berr );
static void init_work( lapack_int size, lapack_complex_double *work );
static void init_rwork( lapack_int size, double *rwork );
static int compare_ztbrfs( double *ferr, double *ferr_i, double *berr,
                           double *berr_i, lapack_int info, lapack_int info_i,
                           lapack_int nrhs );

int main(void)
{
    /* Local scalars */
    char uplo, uplo_i;
    char trans, trans_i;
    char diag, diag_i;
    lapack_int n, n_i;
    lapack_int kd, kd_i;
    lapack_int nrhs, nrhs_i;
    lapack_int ldab, ldab_i;
    lapack_int ldab_r;
    lapack_int ldb, ldb_i;
    lapack_int ldb_r;
    lapack_int ldx, ldx_i;
    lapack_int ldx_r;
    lapack_int info, info_i;
    lapack_int i;
    int failed;

    /* Local arrays */
    lapack_complex_double *ab = NULL, *ab_i = NULL;
    lapack_complex_double *b = NULL, *b_i = NULL;
    lapack_complex_double *x = NULL, *x_i = NULL;
    double *ferr = NULL, *ferr_i = NULL;
    double *berr = NULL, *berr_i = NULL;
    lapack_complex_double *work = NULL, *work_i = NULL;
    double *rwork = NULL, *rwork_i = NULL;
    double *ferr_save = NULL;
    double *berr_save = NULL;
    lapack_complex_double *ab_r = NULL;
    lapack_complex_double *b_r = NULL;
    lapack_complex_double *x_r = NULL;

    /* Iniitialize the scalar parameters */
    init_scalars_ztbrfs( &uplo, &trans, &diag, &n, &kd, &nrhs, &ldab, &ldb,
                         &ldx );
    ldab_r = n+2;
    ldb_r = nrhs+2;
    ldx_r = nrhs+2;
    uplo_i = uplo;
    trans_i = trans;
    diag_i = diag;
    n_i = n;
    kd_i = kd;
    nrhs_i = nrhs;
    ldab_i = ldab;
    ldb_i = ldb;
    ldx_i = ldx;

    /* Allocate memory for the LAPACK routine arrays */
    ab = (lapack_complex_double *)
        LAPACKE_malloc( ldab*n * sizeof(lapack_complex_double) );
    b = (lapack_complex_double *)
        LAPACKE_malloc( ldb*nrhs * sizeof(lapack_complex_double) );
    x = (lapack_complex_double *)
        LAPACKE_malloc( ldx*nrhs * sizeof(lapack_complex_double) );
    ferr = (double *)LAPACKE_malloc( nrhs * sizeof(double) );
    berr = (double *)LAPACKE_malloc( nrhs * sizeof(double) );
    work = (lapack_complex_double *)
        LAPACKE_malloc( 2*n * sizeof(lapack_complex_double) );
    rwork = (double *)LAPACKE_malloc( n * sizeof(double) );

    /* Allocate memory for the C interface function arrays */
    ab_i = (lapack_complex_double *)
        LAPACKE_malloc( ldab*n * sizeof(lapack_complex_double) );
    b_i = (lapack_complex_double *)
        LAPACKE_malloc( ldb*nrhs * sizeof(lapack_complex_double) );
    x_i = (lapack_complex_double *)
        LAPACKE_malloc( ldx*nrhs * sizeof(lapack_complex_double) );
    ferr_i = (double *)LAPACKE_malloc( nrhs * sizeof(double) );
    berr_i = (double *)LAPACKE_malloc( nrhs * sizeof(double) );
    work_i = (lapack_complex_double *)
        LAPACKE_malloc( 2*n * sizeof(lapack_complex_double) );
    rwork_i = (double *)LAPACKE_malloc( n * sizeof(double) );

    /* Allocate memory for the backup arrays */
    ferr_save = (double *)LAPACKE_malloc( nrhs * sizeof(double) );
    berr_save = (double *)LAPACKE_malloc( nrhs * sizeof(double) );

    /* Allocate memory for the row-major arrays */
    ab_r = (lapack_complex_double *)
        LAPACKE_malloc( (kd+1)*(n+2) * sizeof(lapack_complex_double) );
    b_r = (lapack_complex_double *)
        LAPACKE_malloc( n*(nrhs+2) * sizeof(lapack_complex_double) );
    x_r = (lapack_complex_double *)
        LAPACKE_malloc( n*(nrhs+2) * sizeof(lapack_complex_double) );

    /* Initialize input arrays */
    init_ab( ldab*n, ab );
    init_b( ldb*nrhs, b );
    init_x( ldx*nrhs, x );
    init_ferr( nrhs, ferr );
    init_berr( nrhs, berr );
    init_work( 2*n, work );
    init_rwork( n, rwork );

    /* Backup the ouptut arrays */
    for( i = 0; i < nrhs; i++ ) {
        ferr_save[i] = ferr[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_save[i] = berr[i];
    }

    /* Call the LAPACK routine */
    ztbrfs_( &uplo, &trans, &diag, &n, &kd, &nrhs, ab, &ldab, b, &ldb, x, &ldx,
             ferr, berr, work, rwork, &info );

    /* Initialize input data, call the column-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < ldab*n; i++ ) {
        ab_i[i] = ab[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }
    info_i = LAPACKE_ztbrfs_work( LAPACK_COL_MAJOR, uplo_i, trans_i, diag_i,
                                  n_i, kd_i, nrhs_i, ab_i, ldab_i, b_i, ldb_i,
                                  x_i, ldx_i, ferr_i, berr_i, work_i, rwork_i );

    failed = compare_ztbrfs( ferr, ferr_i, berr, berr_i, info, info_i, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: column-major middle-level interface to ztbrfs\n" );
    } else {
        printf( "FAILED: column-major middle-level interface to ztbrfs\n" );
    }

    /* Initialize input data, call the column-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < ldab*n; i++ ) {
        ab_i[i] = ab[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }
    info_i = LAPACKE_ztbrfs( LAPACK_COL_MAJOR, uplo_i, trans_i, diag_i, n_i,
                             kd_i, nrhs_i, ab_i, ldab_i, b_i, ldb_i, x_i, ldx_i,
                             ferr_i, berr_i );

    failed = compare_ztbrfs( ferr, ferr_i, berr, berr_i, info, info_i, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: column-major high-level interface to ztbrfs\n" );
    } else {
        printf( "FAILED: column-major high-level interface to ztbrfs\n" );
    }

    /* Initialize input data, call the row-major middle-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < ldab*n; i++ ) {
        ab_i[i] = ab[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }

    LAPACKE_zge_trans( LAPACK_COL_MAJOR, kd+1, n, ab_i, ldab, ab_r, n+2 );
    LAPACKE_zge_trans( LAPACK_COL_MAJOR, n, nrhs, b_i, ldb, b_r, nrhs+2 );
    LAPACKE_zge_trans( LAPACK_COL_MAJOR, n, nrhs, x_i, ldx, x_r, nrhs+2 );
    info_i = LAPACKE_ztbrfs_work( LAPACK_ROW_MAJOR, uplo_i, trans_i, diag_i,
                                  n_i, kd_i, nrhs_i, ab_r, ldab_r, b_r, ldb_r,
                                  x_r, ldx_r, ferr_i, berr_i, work_i, rwork_i );

    failed = compare_ztbrfs( ferr, ferr_i, berr, berr_i, info, info_i, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: row-major middle-level interface to ztbrfs\n" );
    } else {
        printf( "FAILED: row-major middle-level interface to ztbrfs\n" );
    }

    /* Initialize input data, call the row-major high-level
     * interface to LAPACK routine and check the results */
    for( i = 0; i < ldab*n; i++ ) {
        ab_i[i] = ab[i];
    }
    for( i = 0; i < ldb*nrhs; i++ ) {
        b_i[i] = b[i];
    }
    for( i = 0; i < ldx*nrhs; i++ ) {
        x_i[i] = x[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        ferr_i[i] = ferr_save[i];
    }
    for( i = 0; i < nrhs; i++ ) {
        berr_i[i] = berr_save[i];
    }
    for( i = 0; i < 2*n; i++ ) {
        work_i[i] = work[i];
    }
    for( i = 0; i < n; i++ ) {
        rwork_i[i] = rwork[i];
    }

    /* Init row_major arrays */
    LAPACKE_zge_trans( LAPACK_COL_MAJOR, kd+1, n, ab_i, ldab, ab_r, n+2 );
    LAPACKE_zge_trans( LAPACK_COL_MAJOR, n, nrhs, b_i, ldb, b_r, nrhs+2 );
    LAPACKE_zge_trans( LAPACK_COL_MAJOR, n, nrhs, x_i, ldx, x_r, nrhs+2 );
    info_i = LAPACKE_ztbrfs( LAPACK_ROW_MAJOR, uplo_i, trans_i, diag_i, n_i,
                             kd_i, nrhs_i, ab_r, ldab_r, b_r, ldb_r, x_r, ldx_r,
                             ferr_i, berr_i );

    failed = compare_ztbrfs( ferr, ferr_i, berr, berr_i, info, info_i, nrhs );
    if( failed == 0 ) {
        printf( "PASSED: row-major high-level interface to ztbrfs\n" );
    } else {
        printf( "FAILED: row-major high-level interface to ztbrfs\n" );
    }

    /* Release memory */
    if( ab != NULL ) {
        LAPACKE_free( ab );
    }
    if( ab_i != NULL ) {
        LAPACKE_free( ab_i );
    }
    if( ab_r != NULL ) {
        LAPACKE_free( ab_r );
    }
    if( b != NULL ) {
        LAPACKE_free( b );
    }
    if( b_i != NULL ) {
        LAPACKE_free( b_i );
    }
    if( b_r != NULL ) {
        LAPACKE_free( b_r );
    }
    if( x != NULL ) {
        LAPACKE_free( x );
    }
    if( x_i != NULL ) {
        LAPACKE_free( x_i );
    }
    if( x_r != NULL ) {
        LAPACKE_free( x_r );
    }
    if( ferr != NULL ) {
        LAPACKE_free( ferr );
    }
    if( ferr_i != NULL ) {
        LAPACKE_free( ferr_i );
    }
    if( ferr_save != NULL ) {
        LAPACKE_free( ferr_save );
    }
    if( berr != NULL ) {
        LAPACKE_free( berr );
    }
    if( berr_i != NULL ) {
        LAPACKE_free( berr_i );
    }
    if( berr_save != NULL ) {
        LAPACKE_free( berr_save );
    }
    if( work != NULL ) {
        LAPACKE_free( work );
    }
    if( work_i != NULL ) {
        LAPACKE_free( work_i );
    }
    if( rwork != NULL ) {
        LAPACKE_free( rwork );
    }
    if( rwork_i != NULL ) {
        LAPACKE_free( rwork_i );
    }

    return 0;
}

/* Auxiliary function: ztbrfs scalar parameters initialization */
static void init_scalars_ztbrfs( char *uplo, char *trans, char *diag,
                                 lapack_int *n, lapack_int *kd,
                                 lapack_int *nrhs, lapack_int *ldab,
                                 lapack_int *ldb, lapack_int *ldx )
{
    *uplo = 'L';
    *trans = 'N';
    *diag = 'N';
    *n = 4;
    *kd = 2;
    *nrhs = 2;
    *ldab = 9;
    *ldb = 8;
    *ldx = 8;

    return;
}

/* Auxiliary functions: ztbrfs array parameters initialization */
static void init_ab( lapack_int size, lapack_complex_double *ab ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        ab[i] = lapack_make_complex_double( 0.0, 0.0 );
    }
    ab[0] = lapack_make_complex_double( -1.93999999999999990e+000,
                                        4.42999999999999970e+000 );
    ab[9] = lapack_make_complex_double( 4.12000000000000010e+000,
                                        -4.26999999999999960e+000 );
    ab[18] = lapack_make_complex_double( 4.29999999999999990e-001,
                                         -2.66000000000000010e+000 );
    ab[27] = lapack_make_complex_double( 4.40000000000000000e-001,
                                         1.00000000000000010e-001 );
    ab[1] = lapack_make_complex_double( -3.39000000000000010e+000,
                                        3.43999999999999990e+000 );
    ab[10] = lapack_make_complex_double( -1.84000000000000010e+000,
                                         5.53000000000000020e+000 );
    ab[19] = lapack_make_complex_double( 1.74000000000000000e+000,
                                         -4.00000000000000010e-002 );
    ab[28] = lapack_make_complex_double( 0.00000000000000000e+000,
                                         0.00000000000000000e+000 );
    ab[2] = lapack_make_complex_double( 1.62000000000000010e+000,
                                        3.68000000000000020e+000 );
    ab[11] = lapack_make_complex_double( -2.77000000000000000e+000,
                                         -1.92999999999999990e+000 );
    ab[20] = lapack_make_complex_double( 0.00000000000000000e+000,
                                         0.00000000000000000e+000 );
    ab[29] = lapack_make_complex_double( 0.00000000000000000e+000,
                                         0.00000000000000000e+000 );
}
static void init_b( lapack_int size, lapack_complex_double *b ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        b[i] = lapack_make_complex_double( 0.0, 0.0 );
    }
    b[0] = lapack_make_complex_double( -8.85999999999999940e+000,
                                       -3.87999999999999990e+000 );
    b[8] = lapack_make_complex_double( -2.40900000000000000e+001,
                                       -5.26999999999999960e+000 );
    b[1] = lapack_make_complex_double( -1.55700000000000000e+001,
                                       -2.34100000000000000e+001 );
    b[9] = lapack_make_complex_double( -5.79699999999999990e+001,
                                       8.14000000000000060e+000 );
    b[2] = lapack_make_complex_double( -7.62999999999999990e+000,
                                       2.27800000000000010e+001 );
    b[10] = lapack_make_complex_double( 1.90900000000000000e+001,
                                        -2.95100000000000020e+001 );
    b[3] = lapack_make_complex_double( -1.47400000000000000e+001,
                                       -2.39999999999999990e+000 );
    b[11] = lapack_make_complex_double( 1.91700000000000020e+001,
                                        2.13299999999999980e+001 );
}
static void init_x( lapack_int size, lapack_complex_double *x ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        x[i] = lapack_make_complex_double( 0.0, 0.0 );
    }
    x[0] = lapack_make_complex_double( 0.00000000000000000e+000,
                                       2.00000000000000000e+000 );
    x[8] = lapack_make_complex_double( 1.00000000000000020e+000,
                                       5.00000000000000090e+000 );
    x[1] = lapack_make_complex_double( 9.99999999999999780e-001,
                                       -3.00000000000000000e+000 );
    x[9] = lapack_make_complex_double( -7.00000000000000090e+000,
                                       -1.99999999999999890e+000 );
    x[2] = lapack_make_complex_double( -3.99999999999999960e+000,
                                       -5.00000000000000000e+000 );
    x[10] = lapack_make_complex_double( 2.99999999999999960e+000,
                                        4.00000000000000270e+000 );
    x[3] = lapack_make_complex_double( 1.99999999999999400e+000,
                                       -1.00000000000000310e+000 );
    x[11] = lapack_make_complex_double( -6.00000000000000980e+000,
                                        -9.00000000000000530e+000 );
}
static void init_ferr( lapack_int size, double *ferr ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        ferr[i] = 0;
    }
}
static void init_berr( lapack_int size, double *berr ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        berr[i] = 0;
    }
}
static void init_work( lapack_int size, lapack_complex_double *work ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        work[i] = lapack_make_complex_double( 0.0, 0.0 );
    }
}
static void init_rwork( lapack_int size, double *rwork ) {
    lapack_int i;
    for( i = 0; i < size; i++ ) {
        rwork[i] = 0;
    }
}

/* Auxiliary function: C interface to ztbrfs results check */
/* Return value: 0 - test is passed, non-zero - test is failed */
static int compare_ztbrfs( double *ferr, double *ferr_i, double *berr,
                           double *berr_i, lapack_int info, lapack_int info_i,
                           lapack_int nrhs )
{
    lapack_int i;
    int failed = 0;
    for( i = 0; i < nrhs; i++ ) {
        failed += compare_doubles(ferr[i],ferr_i[i]);
    }
    for( i = 0; i < nrhs; i++ ) {
        failed += compare_doubles(berr[i],berr_i[i]);
    }
    failed += (info == info_i) ? 0 : 1;
    if( info != 0 || info_i != 0 ) {
        printf( "info=%d, info_i=%d\n",(int)info,(int)info_i );
    }

    return failed;
}
