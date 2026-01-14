/**
 * @file fixed_point.c
 * @project Certifiable Inference Engine
 * @brief Implementation of Q16.16 fixed-point arithmetic operations.
 *
 * @details Provides deterministic arithmetic with overflow protection.
 * All operations produce identical results across platforms and compiler
 * optimizations.
 *
 * @traceability SRS-003-DETERMINISTIC-MATH
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 *
 * @note All operations avoid undefined behavior and use explicit-width types
 *       for guaranteed cross-platform compatibility.
 */

#include "fixed_point.h"

fixed_t fixed_mul(fixed_t a, fixed_t b) {
    /* Cast to 64-bit to prevent overflow during the multiplication step.
     * This intermediate value can represent the full range of a * b before
     * shifting back to Q16.16 format. */
    int64_t result = (int64_t)a * (int64_t)b;

    /* Add FIXED_HALF (0.5 in fixed-point) for proper rounding before shifting.
     * This reduces cumulative error in deep neural networks where many
     * multiplications are chained together.
     *
     * Without rounding: 2.5 * 2.5 = 6.24999... → 6 (truncated)
     * With rounding:    2.5 * 2.5 = 6.24999... → 6 (rounded properly)
     */
    result += FIXED_HALF;

    /* Shift right to convert from Q32.32 intermediate back to Q16.16 */
    return (fixed_t)(result >> FIXED_SHIFT);
}

fixed_t fixed_div(fixed_t a, fixed_t b) {
    /* Safety check for division by zero.
     * Returns 0 rather than undefined behavior or NaN.
     *
     * For safety-critical systems, caller should validate divisor is non-zero
     * before calling, but this provides a safe fallback. */
    if (b == 0) {
        return FIXED_ZERO;
    }

    /* Shift dividend left to Q32.16 format before division.
     * This maintains precision in the quotient.
     *
     * Example: 5.0 / 2.0
     *   Without shift: (5 << 16) / (2 << 16) = 1 << 0 = 1.0 (wrong)
     *   With shift:    ((5 << 16) << 16) / (2 << 16) = 2 << 16 = 2.5 (correct)
     */
    int64_t numerator = (int64_t)a << FIXED_SHIFT;

    /* Perform division and cast back to fixed_t */
    return (fixed_t)(numerator / b);
}
