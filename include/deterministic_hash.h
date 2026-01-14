/**
 * @file deterministic_hash.h
 * @brief Bounded, bit-perfect hash table for safety-critical AI pipelines.
 * @copyright Copyright (c) 2026 SpeyTech. All rights reserved.
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
    D_TABLE_OK = 0,
    D_TABLE_FULL,
    D_TABLE_KEY_EXISTS,
    D_TABLE_NOT_FOUND,
    D_TABLE_INVALID_PARAM
} d_table_res_t;

/**
 * @brief Entry structure.
 * Fixed-size keys ensure deterministic memory alignment.
 */
typedef struct {
    char key[32];
    int32_t value;
    bool occupied;
} d_entry_t;

/**
 * @brief The Deterministic Table handle.
 * No dynamic allocation: memory provided by caller.
 */
typedef struct {
    d_entry_t* entries;
    size_t capacity;
    size_t count;
    uint32_t (*hash_fn)(const char* key);
} d_table_t;

/**
 * @brief Initialize the table using a pre-allocated buffer.
 * 
 * Precondition: table and buffer are valid pointers, buffer_size > 0
 * Postcondition: Table initialized and ready for use
 * Complexity: O(n) where n = buffer_size / sizeof(d_entry_t)
 * Determinism: Always produces same initial state
 * 
 * @param table Pointer to table structure
 * @param buffer Pointer to pre-allocated memory pool
 * @param buffer_size Total size of the pool in bytes
 * @return D_TABLE_OK on success, error code otherwise
 */
d_table_res_t d_table_init(d_table_t* table, void* buffer, size_t buffer_size);

/**
 * @brief Insert a key-value pair.
 * 
 * Precondition: table initialized, key is valid string
 * Postcondition: Key-value pair inserted or error returned
 * Complexity: O(1) average, O(n) worst case
 * Determinism: Collision resolution via linear probing is deterministic
 * 
 * @param table Pointer to table
 * @param key Key string (max 31 chars)
 * @param value Integer value to store
 * @return D_TABLE_OK on success, error code otherwise
 */
d_table_res_t d_table_insert(d_table_t* table, const char* key, int32_t value);

/**
 * @brief Retrieve a value by key.
 * 
 * Precondition: table initialized, key and out_value are valid pointers
 * Postcondition: Value retrieved or NOT_FOUND returned
 * Complexity: O(1) average, O(n) worst case
 * Determinism: Always returns same result for same key
 * 
 * @param table Pointer to table
 * @param key Key string to look up
 * @param out_value Pointer to store retrieved value
 * @return D_TABLE_OK on success, D_TABLE_NOT_FOUND if key doesn't exist
 */
d_table_res_t d_table_get(const d_table_t* table, const char* key, int32_t* out_value);

/**
 * @brief Deterministic iteration over all entries.
 * 
 * Precondition: table and callback are valid pointers
 * Postcondition: Callback invoked for each occupied entry
 * Complexity: O(n) where n = capacity
 * Determinism: Iteration order based on entry index, not memory addresses
 * 
 * @param table Pointer to table
 * @param callback Function to call for each entry
 */
void d_table_iterate(const d_table_t* table, void (*callback)(const char* key, int32_t value));

#endif // DETERMINISTIC_HASH_H
