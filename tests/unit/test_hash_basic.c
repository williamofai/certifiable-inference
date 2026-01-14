/**
 * @file test_hash_basic.c
 * @brief Basic unit tests for deterministic hash table.
 */

#include "deterministic_hash.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_init(void) {
    uint8_t buffer[1024];
    d_table_t table;
    
    d_table_res_t result = d_table_init(&table, buffer, sizeof(buffer));
    assert(result == D_TABLE_OK);
    assert(table.count == 0);
    assert(table.capacity > 0);
    
    printf("✓ test_init passed\n");
}

void test_insert_and_get(void) {
    uint8_t buffer[1024];
    d_table_t table;
    
    d_table_init(&table, buffer, sizeof(buffer));
    
    // Insert
    d_table_res_t result = d_table_insert(&table, "test_key", 42);
    assert(result == D_TABLE_OK);
    assert(table.count == 1);
    
    // Get
    int32_t value;
    result = d_table_get(&table, "test_key", &value);
    assert(result == D_TABLE_OK);
    assert(value == 42);
    
    printf("✓ test_insert_and_get passed\n");
}

void test_duplicate_key(void) {
    uint8_t buffer[1024];
    d_table_t table;
    
    d_table_init(&table, buffer, sizeof(buffer));
    
    d_table_insert(&table, "key1", 10);
    d_table_res_t result = d_table_insert(&table, "key1", 20);
    
    assert(result == D_TABLE_KEY_EXISTS);
    
    printf("✓ test_duplicate_key passed\n");
}

void test_not_found(void) {
    uint8_t buffer[1024];
    d_table_t table;
    
    d_table_init(&table, buffer, sizeof(buffer));
    
    int32_t value;
    d_table_res_t result = d_table_get(&table, "nonexistent", &value);
    
    assert(result == D_TABLE_NOT_FOUND);
    
    printf("✓ test_not_found passed\n");
}

static int callback_count = 0;
static void count_callback(const char* key, int32_t value) {
    (void)key;
    (void)value;
    callback_count++;
}

void test_iterate(void) {
    uint8_t buffer[1024];
    d_table_t table;
    
    d_table_init(&table, buffer, sizeof(buffer));
    
    d_table_insert(&table, "key1", 1);
    d_table_insert(&table, "key2", 2);
    d_table_insert(&table, "key3", 3);
    
    callback_count = 0;
    d_table_iterate(&table, count_callback);
    
    assert(callback_count == 3);
    
    printf("✓ test_iterate passed\n");
}

int main(void) {
    printf("Running deterministic hash table tests...\n\n");
    
    test_init();
    test_insert_and_get();
    test_duplicate_key();
    test_not_found();
    test_iterate();
    
    printf("\n✅ All tests passed!\n");
    return 0;
}
