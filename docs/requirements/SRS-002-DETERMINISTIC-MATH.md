# SRS-002: Deterministic Fixed-Point Arithmetic

| Field | Value |
|-------|-------|
| **ID** | SRS-002 |
| **Component** | Core / Math |
| **Status** | Implemented |
| **Compliance** | ISO 26262-6, IEC 62304 |
| **Applicability** | Safety-Critical Inference |

## 1. Purpose

The system shall provide a mathematical foundation that guarantees bit-identical results across heterogeneous hardware architectures (x86, ARM, RISC-V). This eliminates the non-determinism inherent in IEEE-754 floating-point implementations.

## 2. Requirements

### 2.1 Format Specification

**SRS-002.1:** The system shall utilize a signed Q16.16 fixed-point format stored in a 32-bit integer (`int32_t`).

**Rationale:** Q16.16 format provides sufficient precision for neural network weights and activations while maintaining exact integer arithmetic properties.

**SRS-002.2:** The system shall support a range of [-32768, 32767.99998] with precision of 0.000015 (1/65536).

**Rationale:** This range covers typical neural network parameter values. Values requiring larger range must be scaled at model quantization time.

### 2.2 Functional Requirements

**SRS-002.3: Bit-Perfect Parity**

All arithmetic operations shall produce identical bit patterns regardless of:
- Compiler optimization levels (`-O0` to `-Ofast`)
- SIMD instruction availability
- Target architecture (x86, ARM, RISC-V)
- Compiler vendor (GCC, Clang, MSVC)

**Rationale:** Safety-critical certification requires reproducible behavior. Floating-point operations vary across these parameters, making certification impossible.

**Verification:** Cross-platform testing with checksum comparison.

---

**SRS-002.4: Rounding**

Multiplication operations shall utilize "round to nearest" (adding 0.5 LSB before shifting) to minimize cumulative quantization error in deep neural networks.

**Rationale:** Truncation accumulates negative bias in multi-layer networks. Proper rounding reduces error propagation.

**Mathematical Definition:**
```
result = (a * b + FIXED_HALF) >> FIXED_SHIFT
```

Where `FIXED_HALF = 1 << (FIXED_SHIFT - 1)` represents 0.5 in fixed-point format.

**Verification:** Unit tests comparing rounded vs truncated results over 10,000 operations.

---

**SRS-002.5: Overflow Protection**

Multiplication shall utilize 64-bit intermediate accumulators to prevent overflow during the calculation phase.

**Rationale:**
- 32-bit multiplication: `(a << 16) * (b << 16)` overflows at small values
- 64-bit intermediate: `((int64_t)a * (int64_t)b)` safely handles full Q16.16 range

**Example:**
- Without protection: `180.0 * 180.0` overflows (both operands valid, product valid, but intermediate overflows)
- With protection: Correct result `32400.0`

**Verification:** Torture tests with boundary values.

### 2.3 Safety & Constraints

**SRS-002.6: Division by Zero**

The system shall handle division by zero as a defined state (returning 0) to prevent processor exceptions or undefined behavior.

**Rationale:**
- IEEE-754 floating-point returns NaN (non-deterministic)
- Integer division by zero triggers hardware exception (undefined behavior)
- Returning 0 provides safe, deterministic fallback

**Note:** For safety-critical use, caller should validate divisor before calling. This provides defense-in-depth.

**Verification:** Unit test confirms `fx_div(x, 0) == 0` for all x.

---

**SRS-002.7: No Hidden State**

Mathematical functions shall be pure (no global state) and re-entrant.

**Rationale:**
- Global state introduces non-determinism
- Non-reentrant code prevents parallel execution
- Pure functions enable formal verification

**Implementation Requirements:**
- No static variables in function scope
- No global variables accessed
- No modification of input parameters
- Thread-safe by design

**Verification:** Static analysis with cppcheck, manual code review.

## 3. Verification Criteria

**V-002.1: Cross-Platform Parity**

Unit tests shall compare fixed-point results against a 64-bit reference implementation on at least two different CPU architectures.

**Test Matrix:**
- x86_64 (Ubuntu 24.04, GCC 13.2)
- ARM Cortex-A (Raspberry Pi, GCC 13.2)
- RISC-V (QEMU emulation, GCC 13.2)

**Pass Criteria:** Bit-identical results (checksum match) for all test vectors.

---

**V-002.2: No Floating-Point Instructions**

Static analysis must verify that no floating-point registers or instructions are utilized in the `src/core/fixed_point.c` module during runtime operations.

**Method:**
- Disassemble compiled binary
- Verify no FPU instructions (e.g., `fadd`, `fmul`, `movss`, `vmovaps`)
- Conversion functions (`fixed_from_float`) permitted for initialization only

**Tools:** `objdump -d`, manual inspection.

---

**V-002.3: Rounding Accuracy**

Multiplication with rounding shall produce results within 0.5 ULP (Unit in Last Place) of exact mathematical result.

**Test:**
- Generate 10,000 random Q16.16 pairs
- Compare `fx_mul(a, b)` against high-precision reference
- Verify error ≤ 0.5 * (1/65536)

---

**V-002.4: Overflow Torture Tests**

Test suite shall include boundary cases that would overflow 32-bit intermediates:
- Maximum positive values: `32767.0 * 32767.0`
- Mixed signs: `-32768.0 * 32767.0`
- Edge cases: `180.0 * 180.0` (overflows 32-bit, fits 64-bit)

**Pass Criteria:** All tests produce mathematically correct results.

## 4. Implementation

**Files:**
- `include/fixed_point.h` - API specification
- `src/core/fixed_point.c` - Implementation
- `tests/unit/test_fixed_point.c` - Verification suite

**Traceability:**
- Code: `@traceability SRS-002-DETERMINISTIC-MATH`
- Tests: Link to this document in header

## 5. Design Rationale

### Why Q16.16?

**Alternative formats considered:**
- Q15.16: Smaller range, no advantage
- Q8.24: More precision, insufficient range for typical NN weights
- Q31.32 (64-bit): More range/precision, but 2x memory and slower on 32-bit MCUs

**Decision:** Q16.16 optimal for embedded safety-critical ML inference.

### Why Not IEEE-754?

| Property | IEEE-754 Float | Q16.16 Fixed |
|----------|----------------|--------------|
| Deterministic | ❌ (platform-dependent) | ✅ |
| Certifiable | ❌ (non-reproducible) | ✅ |
| Performance (32-bit) | Slower (FPU) | Faster (ALU) |
| Memory | 4 bytes | 4 bytes |
| Range | ±10^38 | ±32768 |
| Precision | ~7 decimal | ~5 decimal |

For neural networks with quantized weights, Q16.16 precision is sufficient.

## 6. Future Extensions

**SRS-002.8:** (Planned) Saturating Arithmetic

Provide optional saturating variants that clamp to `FIXED_MIN`/`FIXED_MAX` on overflow rather than wrapping.

**Use case:** Additional safety layer for aerospace applications.

---

**SRS-002.9:** (Planned) Q8.8 Format

Add support for lower-precision Q8.8 format for memory-constrained applications.

**Trade-off:** 2x memory savings, reduced precision.

## 7. References

- **MISRA-C:2012** - Rule 10.1 (Operands shall not be of inappropriate type)
- **ISO 26262-6:2018** - Software unit design and implementation
- **IEC 62304:2006** - Medical device software lifecycle
- **DO-178C** - Software considerations in airborne systems

## 8. Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-01-14 | SpeyTech | Initial version |

---

**Document Classification:** Technical Specification  
**Approval Status:** Approved for Implementation  
**Next Review:** 2026-04-14
