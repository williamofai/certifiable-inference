/**
 * @file matrix.c
 * @project Certifiable Inference Engine
 * @brief Implementation of deterministic matrix operations.
 *
 * @details Provides bit-perfect linear algebra primitives for neural network
 * inference. Uses 64-bit accumulators and sequential operations to guarantee
 * reproducibility across all platforms.
 *
 * @traceability SRS-003-LINEAR-ALGEBRA
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#include "matrix.h"
#include <string.h>

void fx_matrix_init(fx_matrix_t* mat, fixed_t* buffer, uint16_t rows, uint16_t cols) {
    if (!mat || !buffer) {
        return;
    }

    mat->data = buffer;
    mat->rows = rows;
    mat->cols = cols;

    /* Ensure memory is clean for determinism (SRS-003.1) */
    memset(mat->data, 0, (size_t)rows * cols * sizeof(fixed_t));
}

void fx_matrix_mul(const fx_matrix_t* A, const fx_matrix_t* B, fx_matrix_t* C) {
    /* SRS-003.4: Dimensional validation - safety first */
    if (!A || !B || !C) {
        return;
    }

    if (A->cols != B->rows) {
        /* Incompatible dimensions - safe failure mode */
        return;
    }

    /* SRS-003.6: Bounded execution O(N*M*P) with no data-dependent branching */
    for (uint16_t i = 0; i < A->rows; i++) {
        for (uint16_t j = 0; j < B->cols; j++) {
            /* SRS-003.5: 64-bit accumulator prevents overflow */
            int64_t sum = 0;

            /* Inner loop: dot product of row i of A with column j of B */
            for (uint16_t k = 0; k < A->cols; k++) {
                /* SRS-003.2: Row-major access for cache efficiency
                 * A[i][k] = A.data[i * A.cols + k]
                 * B[k][j] = B.data[k * B.cols + j] */
                fixed_t val_a = A->data[i * A->cols + k];
                fixed_t val_b = B->data[k * B->cols + j];

                /* Multiply without intermediate quantization
                 * Product is Q32.32 (int64_t) */
                int64_t prod = (int64_t)val_a * val_b;
                sum += prod;
            }

            /* SRS-003.4: Quantize back to Q16.16 with proper rounding
             * Add FIXED_HALF (0.5) before shifting for round-to-nearest */
            sum += FIXED_HALF;
            C->data[i * C->cols + j] = (fixed_t)(sum >> FIXED_SHIFT);
        }
    }
}

fixed_t fx_vector_dot(const fixed_t* a, const fixed_t* b, uint16_t len) {
    if (!a || !b) {
        return FIXED_ZERO;
    }

    /* SRS-003.5: 64-bit accumulator for overflow protection */
    int64_t sum = 0;

    /* SRS-003.6: Sequential iteration, no data-dependent branching */
    for (uint16_t i = 0; i < len; i++) {
        int64_t prod = (int64_t)a[i] * b[i];
        sum += prod;
    }

    /* Round and quantize back to fixed-point */
    sum += FIXED_HALF;
    return (fixed_t)(sum >> FIXED_SHIFT);
}

void fx_matrix_add(const fx_matrix_t* A, const fx_matrix_t* B, fx_matrix_t* C) {
    /* Dimension validation */
    if (!A || !B || !C) {
        return;
    }

    if (A->rows != B->rows || A->cols != B->cols) {
        return; /* Incompatible dimensions */
    }

    if (C->rows != A->rows || C->cols != A->cols) {
        return; /* Output dimension mismatch */
    }

    /* Element-wise addition */
    uint16_t total_elements = A->rows * A->cols;
    for (uint16_t i = 0; i < total_elements; i++) {
        C->data[i] = fixed_add(A->data[i], B->data[i]);
    }
}

void fx_matrix_apply(fx_matrix_t* mat, fixed_t (*fn)(fixed_t)) {
    if (!mat || !fn) {
        return;
    }

    /* Apply function to each element */
    uint16_t total_elements = mat->rows * mat->cols;
    for (uint16_t i = 0; i < total_elements; i++) {
        mat->data[i] = fn(mat->data[i]);
    }
}
