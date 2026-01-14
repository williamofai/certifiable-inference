# SpeyTech High-Integrity C Style Guide

For safety-critical systems targeting MISRA-C:2012 compliance.

## Fundamental Rules

### Memory Management
- **RULE 1:** No dynamic allocation after initialization
  - ❌ `malloc()`, `free()`, `realloc()` forbidden in runtime code
  - ✅ Pre-allocated buffers passed to `_init()` functions

### Type Safety (MISRA-C Rule 4.6)
- **RULE 2:** Use explicit-width types
  - ❌ `int`, `long`, `unsigned`
  - ✅ `int32_t`, `uint32_t`, `size_t`

### Determinism
- **RULE 3:** No undefined behavior
  - ❌ Variable-length arrays
  - ❌ Pointer arithmetic beyond array indexing
  - ❌ Recursion (use iteration)

### Naming Conventions
- Functions: `snake_case` (e.g., `d_table_init`)
- Types: `snake_case_t` (e.g., `d_table_t`)
- Macros: `UPPER_CASE` (e.g., `MAX_CAPACITY`)
- Constants: `UPPER_CASE` (e.g., `HASH_SEED`)

### Documentation (MISRA-C Rule 3.1)
Every function must document:
- **Purpose:** What it does
- **Preconditions:** What must be true before calling
- **Postconditions:** What is guaranteed after calling
- **Complexity:** O(1), O(n), etc.
- **Determinism:** Reproducibility guarantee

## Code Example (Compliant)

```c
#include <stdint.h>

/**
 * @brief Process sensor reading with bounds checking.
 * 
 * Precondition: reading is valid pointer
 * Postcondition: Returns normalized value in range [0, 1000]
 * Complexity: O(1)
 * Determinism: Bit-perfect
 */
int32_t process_reading(const sensor_reading_t* reading) {
    if (reading == NULL) {
        return -1;
    }
    
    if (reading->value > 1000) {
        return 1000;
    }
    
    return reading->value;
}
```

## Code Example (Non-Compliant)

```c
// Bad: Multiple violations
int process(void* data) {  // Undefined width, void*
    int* arr = malloc(100);  // Dynamic allocation
    
    for (int i = 0; i < 100; i++) {
        arr[i] = rand();  // Non-deterministic
    }
    
    free(arr);
    return 0;
}
```
