# SRS-003: Deterministic Linear Algebra

| Field | Value |
|-------|-------|
| **ID** | SRS-003 |
| **Component** | Core / Matrix |
| **Status** | In Progress |
| **Compliance** | ISO 26262, MISRA-C:2012 |
| **Applicability** | Neural Network Layers (Dense, Conv2D) |

## 1. Purpose

This module provides the linear algebra primitives required for neural network inference. It must ensure that multi-dimensional array operations are performed with bounded memory and bit-perfect consistency across all supported hardware platforms.

## 2. Requirements

### 2.1 Memory Management

**SRS-003.1: No Dynamic Allocation**

Matrix structures shall be initialized using caller-provided memory buffers. The module shall not invoke `malloc()`, `calloc()`, or `realloc()`.

**Rationale:** Dynamic allocation introduces non-determinism through heap fragmentation and ASLR (Address Space Layout Randomization). Pre-allocated buffers ensure predictable memory layout.

**Verification:** Static analysis confirms no malloc family functions called.

---

**SRS-003.2: Spatial Locality**

Matrix data shall be stored in row-major order to optimize cache hits during standard dot-product operations.

**Rationale:** Row-major layout (C convention) ensures sequential memory access during matrix multiplication inner loops, maximizing cache efficiency.

**Layout:**
```
Matrix[i][j] → data[i * cols + j]
```

**Verification:** Code inspection confirms row-major indexing.

### 2.2 Functional Requirements

**SRS-003.3: GEMM Determinism**

Matrix multiplication (C = A × B) must produce identical bit patterns regardless of the underlying CPU's SIMD (Single Instruction, Multiple Data) capabilities.

**Rationale:**
- Standard BLAS libraries reorder operations for performance
- SIMD instruction sets (SSE, AVX, NEON) vary by platform
- Operation reordering changes accumulation order, affecting results

**Implementation:** Sequential scalar operations only, no SIMD intrinsics.

**Verification:** Cross-platform bit-perfect testing (x86 vs ARM).

---

**SRS-003.4: Dimensional Validation**

The system shall verify that `A_cols == B_rows` before execution. Failure to meet this condition must result in a defined error state rather than a memory corruption.

**Rationale:** Dimension mismatches are a common source of buffer overflows in matrix code.

**Safe Failure:** Function returns early without modifying output matrix.

**Verification:** Unit test confirms rejected operations don't corrupt memory.

---

**SRS-003.5: Accumulator Precision**

Dot-product intermediate sums shall use 64-bit wide accumulators (`int64_t`) to prevent overflow before the final quantization back to 32-bit fixed-point.

**Rationale:**

Example calculation showing overflow risk:
```
Dot product: sum = Σ(a[i] * b[i])
Each a[i], b[i] is Q16.16 (int32_t)
Product: int32_t * int32_t = needs int64_t
Sum of 1000 products: definitely needs int64_t
```

Without 64-bit accumulator:
- 100 multiplications of max values overflow 32-bit sum
- Neural networks often have 1000+ weights per layer

**Verification:** Torture test with large matrices and max values.

### 2.3 Performance and Safety

**SRS-003.6: Bounded Execution**

The execution time of a matrix multiplication must be strictly O(N·M·P) where N, M, P are dimensions. No data-dependent branching is permitted within the inner loops.

**Rationale:**
- Data-dependent timing enables side-channel attacks
- Predictable execution time required for real-time systems
- Simplifies worst-case execution time (WCET) analysis

**Implementation:** No conditional statements in multiplication loops.

**Verification:** Timing analysis shows constant time per element.

## 3. Verification Criteria

**V-003.1: Cross-Platform Consistency**

Compare C matrix results between:
- Optimized build (`-O3`) and debug build (`-O0`)
- x86_64 and ARM architectures
- GCC and Clang compilers

**Pass Criteria:** Bit-for-bit identical results across all configurations.

---

**V-003.2: Address Independence**

Verify that swapping the memory addresses of two identical matrices does not change the resulting product.

**Test Method:**
1. Run matrix multiplication with matrices at addresses X, Y
2. Copy matrices to different addresses X', Y'
3. Run multiplication again
4. Compare results byte-for-byte

**Pass Criteria:** Results identical regardless of memory location.

---

**V-003.3: Dimension Safety**

Test multiplication with incompatible dimensions confirms:
- No crash or undefined behavior
- Output matrix unmodified
- Predictable failure mode

**Pass Criteria:** All invalid dimension combinations handled safely.

## 4. Design Rationale

### Why Not Use BLAS?

| Property | BLAS (OpenBLAS/MKL) | This Implementation |
|----------|---------------------|---------------------|
| Deterministic | ❌ (operation reordering) | ✅ |
| Certifiable | ❌ (non-reproducible) | ✅ |
| Performance | Faster (optimized) | Predictable (bounded) |
| Dependencies | External library | Zero (pure C99) |
| SIMD | Yes (platform-specific) | No (portable) |

**Decision:** Performance matters less than correctness for safety-critical certification.

### Memory Layout Trade-offs

**Row-Major (Selected):**
- ✅ Standard C convention
- ✅ Cache-friendly for A·B multiplication
- ✅ Simple indexing: `data[i * cols + j]`

**Column-Major (Rejected):**
- ❌ Requires stride calculations
- ❌ Cache-unfriendly for typical operations
- ❌ Used by Fortran/MATLAB (not our target)

## 5. Implementation

**Files:**
- `include/matrix.h` - API specification
- `src/core/matrix.c` - Implementation
- `tests/unit/test_matrix_reproducibility.c` - Verification

**Key Functions:**
```c
fx_matrix_init()  // Initialize with pre-allocated buffer
fx_matrix_mul()   // C = A × B (GEMM)
fx_vector_dot()   // Dot product for dense layers
```

**Traceability:**
- Code: `@traceability SRS-003-LINEAR-ALGEBRA`
- Tests: Link to this document in header

## 6. Usage Example

```c
// Pre-allocate buffers (no malloc)
fixed_t buf_a[3 * 3];
fixed_t buf_b[3 * 3];
fixed_t buf_c[3 * 3];

// Initialize matrices
fx_matrix_t A, B, C;
fx_matrix_init(&A, buf_a, 3, 3);
fx_matrix_init(&B, buf_b, 3, 3);
fx_matrix_init(&C, buf_c, 3, 3);

// Fill with values
A.data[0] = fixed_from_float(1.5f);
// ... fill rest of A and B

// Multiply (bit-perfect, deterministic)
fx_matrix_mul(&A, &B, &C);

// Result in C.data[], ready for next layer
```

## 7. Performance Characteristics

**Time Complexity:**
- Matrix multiplication: O(N·M·P)
- Vector dot product: O(N)
- Initialization: O(N·M)

**Space Complexity:**
- O(1) - No dynamic allocation
- Memory usage determined by caller-provided buffers

**Cache Behavior:**
- Row-major layout optimizes for sequential access
- Inner loop stride = sizeof(fixed_t)

## 8. Future Enhancements

**SRS-003.7:** (Planned) Strided Matrix Operations

Support for sub-matrix views without copying data.

**Use case:** Convolutional layers need sliding window access.

---

**SRS-003.8:** (Planned) Transpose Optimization

Cache-friendly matrix transposition for certain layer types.

**Trade-off:** Adds complexity but improves Conv2D performance.

## 9. References

- **MISRA-C:2012** - Rule 17.7 (Return values shall be checked)
- **ISO 26262-6:2018** - Software unit design and implementation
- **IEC 62304:2006** - Medical device software lifecycle
- **Golub & Van Loan** - Matrix Computations (algorithm reference)

## 10. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-15 | William Murray | Initial version |

---

**Document Classification:** Technical Specification  
**Approval Status:** Approved for Implementation  
**Next Review:** 2026-04-15
