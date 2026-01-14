/**
 * @file deterministic_hash.c
 * @brief Implementation of deterministic hash table.
 * @copyright Copyright (c) 2026 SpeyTech. All rights reserved.
 */

#include "deterministic_hash.h"
#include <string.h>

/**
 * @brief Jenkins One-at-a-Time Hash.
 * 
 * A deterministic hash function that produces the same result on any
 * architecture regardless of endianness or word size for string inputs.
 * 
 * @param key Input string
 * @return 32-bit hash value
 */
static uint32_t jenkins_hash(const char* key) {
    uint32_t hash = 0;
    while (*key) {
        hash += (uint32_t)(*key++);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

d_table_res_t d_table_init(d_table_t* table, void* buffer, size_t buffer_size) {
    if (!table || !buffer) {
        return D_TABLE_INVALID_PARAM;
    }

    table->capacity = buffer_size / sizeof(d_entry_t);
    if (table->capacity == 0) {
        return D_TABLE_INVALID_PARAM;
    }

    table->entries = (d_entry_t*)buffer;
    table->count = 0;
    table->hash_fn = jenkins_hash;

    // Explicitly zero out the memory pool for determinism
    memset(table->entries, 0, buffer_size);

    return D_TABLE_OK;
}

d_table_res_t d_table_insert(d_table_t* table, const char* key, int32_t value) {
    if (!table || !key) {
        return D_TABLE_INVALID_PARAM;
    }
    
    if (table->count >= table->capacity) {
        return D_TABLE_FULL;
    }

    uint32_t hash = table->hash_fn(key);
    size_t index = (size_t)(hash % table->capacity);

    // Linear Probing: Deterministic collision resolution
    while (table->entries[index].occupied) {
        if (strncmp(table->entries[index].key, key, 32) == 0) {
            return D_TABLE_KEY_EXISTS;
        }
        index = (index + 1) % table->capacity;
    }

    // Insert entry
    strncpy(table->entries[index].key, key, 31);
    table->entries[index].key[31] = '\0'; // Ensure null termination
    table->entries[index].value = value;
    table->entries[index].occupied = true;
    table->count++;

    return D_TABLE_OK;
}

d_table_res_t d_table_get(const d_table_t* table, const char* key, int32_t* out_value) {
    if (!table || !key || !out_value) {
        return D_TABLE_INVALID_PARAM;
    }

    uint32_t hash = table->hash_fn(key);
    size_t index = (size_t)(hash % table->capacity);
    size_t start_index = index;

    while (table->entries[index].occupied) {
        if (strncmp(table->entries[index].key, key, 32) == 0) {
            *out_value = table->entries[index].value;
            return D_TABLE_OK;
        }
        index = (index + 1) % table->capacity;
        if (index == start_index) {
            break; // Wrapped around
        }
    }

    return D_TABLE_NOT_FOUND;
}

void d_table_iterate(const d_table_t* table, void (*callback)(const char* key, int32_t value)) {
    if (!table || !callback) {
        return;
    }

    // Iteration is strictly by table index, ensuring the same order 
    // across all runs for a given set of insertions.
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->entries[i].occupied) {
            callback(table->entries[i].key, table->entries[i].value);
        }
    }
}
