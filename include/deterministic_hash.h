/**
 * @file deterministic_hash.h
 * @project Certifiable Inference Engine
 * @brief Bounded, bit-perfect hash table for safety-critical AI pipelines.
 *
 * @details Provides a deterministic hash table with guaranteed iteration order,
 * zero dynamic allocation, and bit-perfect reproducibility across platforms.
 * Designed for integration into safety-critical ML inference pipelines.
 *
 * @traceability SRS-001-DETERMINISM, SRS-002-BOUNDED-MEMORY
 * @compliance MISRA-C:2012, ISO 26262, IEC 62304, DO-178C
 *
 * @author William Murray
 * @copyright Copyright (c) 2026 The Murray Family Innovation Trust. All rights reserved.
 * @license Licensed under the GPL-3.0 (Open Source) or Commercial License.
 *          For commercial licensing: william@fstopify.com
 */

#ifndef DETERMINISTIC_HASH_H
#define DETERMINISTIC_HASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Error codes for table operations.
 */
typedef enum {
    D_TABLE_OK = 0,              /**< Operation successful */
    D_TABLE_FULL,                /**< Table at capacity */
    D_TABLE_KEY_EXISTS,          /**< Key already present */
    D_TABLE_NOT_FOUND,           /**< Key not found */
    D_TABLE_INVALID_PARAM        /**< Invalid parameter */
} d_table_res_t;

/**
 * @brief Entry structure.
 *
 * @note Fixed-size keys (32 bytes) ensure deterministic memory alignment
 *       and eliminate pointer-based string dependencies.
 */
typedef struct {
    char key[32];                /**< Fixed-size key (31 chars + null) */
    int32_t value;               /**< Integer value */
    bool occupied;               /**< Occupied flag */
} d_entry_t;

/**
 * @brief The Deterministic Table handle.
 *
 * @note No dynamic allocation: memory provided by caller ensures
 *       O(1) space complexity and predictable behavior.
 */
typedef struct {
    d_entry_t* entries;          /**< Pre-allocated entry array */
    size_t capacity;             /**< Maximum entries */
    size_t count;                /**< Current entries */
    uint32_t (*hash_fn)(const char* key);  /**< Hash function pointer */
} d_table_t;

/**
 * @brief Initialize the table using a pre-allocated buffer.
 *
 * @details Prepares table for use with provided memory pool. Zeroes all
 * memory to ensure deterministic initial state with no uninitialized data.
 *
 * @param[out] table Pointer to table structure
 * @param[in] buffer Pointer to pre-allocated memory pool
 * @param[in] buffer_size Total size of the pool in bytes
 *
 * @return D_TABLE_OK on success, error code otherwise
 *
 * @pre table and buffer are valid pointers, buffer_size > sizeof(d_entry_t)
 * @post Table initialized and ready for use, all entries zeroed
 *
 * @complexity O(n) where n = buffer_size / sizeof(d_entry_t)
 * @determinism Always produces same initial state for same buffer
 *
 * @traceability SRS-002-BOUNDED-MEMORY
 */
d_table_res_t d_table_init(d_table_t* table, void* buffer, size_t buffer_size);

/**
 * @brief Insert a key-value pair.
 *
 * @details Inserts entry using Jenkins hash and linear probing for collision
 * resolution. Both hash function and probing are deterministic, ensuring
 * bit-perfect behavior across platforms and runs.
 *
 * @param[in,out] table Pointer to table
 * @param[in] key Key string (max 31 chars, will be truncated)
 * @param[in] value Integer value to store
 *
 * @return D_TABLE_OK on success, error code otherwise
 *
 * @pre table initialized, key is valid string
 * @post Key-value pair inserted or error returned, table count updated
 *
 * @complexity O(1) average case, O(n) worst case with full table
 * @determinism Collision resolution via linear probing is deterministic
 *
 * @traceability SRS-001-DETERMINISM
 */
d_table_res_t d_table_insert(d_table_t* table, const char* key, int32_t value);

/**
 * @brief Retrieve a value by key.
 *
 * @details Looks up key using same Jenkins hash and linear probing as insert,
 * guaranteeing consistent lookup behavior.
 *
 * @param[in] table Pointer to table
 * @param[in] key Key string to look up
 * @param[out] out_value Pointer to store retrieved value
 *
 * @return D_TABLE_OK if found, D_TABLE_NOT_FOUND if key doesn't exist
 *
 * @pre table initialized, key and out_value are valid pointers
 * @post Value retrieved if key exists, out_value unchanged otherwise
 *
 * @complexity O(1) average case, O(n) worst case
 * @determinism Always returns same result for same key
 *
 * @traceability SRS-001-DETERMINISM
 */
d_table_res_t d_table_get(const d_table_t* table, const char* key, int32_t* out_value);

/**
 * @brief Deterministic iteration over all entries.
 *
 * @details Iterates strictly by entry index (0 to capacity-1), not by hash
 * order or memory addresses. This ensures identical iteration order across
 * all runs with the same insertion sequence.
 *
 * @param[in] table Pointer to table
 * @param[in] callback Function to call for each entry
 *
 * @pre table and callback are valid pointers
 * @post Callback invoked exactly once for each occupied entry
 *
 * @complexity O(n) where n = capacity
 * @determinism Iteration order based on entry index, platform-independent
 *
 * @traceability SRS-001-DETERMINISM
 */
void d_table_iterate(const d_table_t* table, void (*callback)(const char* key, int32_t value));

#endif /* DETERMINISTIC_HASH_H */
