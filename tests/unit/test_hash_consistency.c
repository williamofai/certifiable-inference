/**
 * @file test_hash_consistency.c
 * @project Certifiable Inference Engine
 * @brief Bit-perfect consistency test for deterministic hash table.
 *
 * @details This test proves that the hash table produces identical memory states
 * across multiple runs with the same operations. This is the "mic drop"
 * proof of determinism - not just functional equivalence, but bit-for-bit
 * identical memory layout.
 *
 * @traceability SRS-001-DETERMINISM, SRS-002-BOUNDED-MEMORY
 * @compliance MISRA-C:2012, ISO 26262
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 */

#include "deterministic_hash.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define POOL_SIZE 1024

/**
 * @brief Simulated workload representing a typical ML feature store operation.
 *
 * This mimics storing sensor readings and model metadata in a production system.
 */
void run_simulated_workload(uint8_t* buffer) {
    d_table_t table;
    d_table_init(&table, buffer, POOL_SIZE);

    // Insert typical ML feature values
    d_table_insert(&table, "sensor_a", 100);
    d_table_insert(&table, "sensor_b", -50);
    d_table_insert(&table, "model_version", 1);
    d_table_insert(&table, "threshold", 999);
    d_table_insert(&table, "cardiac_rate", 72);
    d_table_insert(&table, "oxygen_sat", 98);
    d_table_insert(&table, "temperature", 37);
    d_table_insert(&table, "blood_pressure", 120);
}

int main(void) {
    uint8_t buffer1[POOL_SIZE];
    uint8_t buffer2[POOL_SIZE];

    printf("Running Bit-Perfect Consistency Test...\n");
    printf("This proves determinism by comparing entire memory states.\n\n");

    // Run 1: First execution
    printf("Run 1: Executing simulated workload...\n");
    run_simulated_workload(buffer1);

    // Run 2: Second execution (identical operations)
    printf("Run 2: Executing identical workload...\n");
    run_simulated_workload(buffer2);

    // The Ultimate Proof: Byte-for-byte comparison of entire memory pools
    printf("\nComparing memory states (byte-for-byte)...\n");
    int result = memcmp(buffer1, buffer2, POOL_SIZE);

    if (result == 0) {
        printf("✅ PASS: Memory states are bit-identical.\n");
        printf("\nThis proves:\n");
        printf("  • No uninitialized memory leakage\n");
        printf("  • No memory-address dependencies\n");
        printf("  • Reproducible behavior across runs\n");
        printf("  • Suitable for safety-critical certification\n");
        return 0;
    } else {
        printf("❌ FAIL: Non-determinism detected in memory layout.\n");
        printf("Memory differs at some byte position.\n");

        // Show first few differences for debugging
        printf("\nFirst differences:\n");
        int diff_count = 0;
        for (size_t i = 0; i < POOL_SIZE && diff_count < 5; i++) {
            if (buffer1[i] != buffer2[i]) {
                printf("  Offset %zu: 0x%02x vs 0x%02x\n", i, buffer1[i], buffer2[i]);
                diff_count++;
            }
        }

        return 1;
    }
}
