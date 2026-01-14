# Contributing to Certifiable Inference

Thank you for your interest! We are building the world's most reliable AI inference engine for safety-critical systems.

## 1. The Legal Bit (CLA)

All contributors must sign our **Contributor License Agreement (CLA)**.

**Why?** It allows SpeyTech to provide commercial licenses to companies that cannot use GPL code while keeping the project open source.

**How?** Our [CLA Assistant](https://cla-assistant.io/) will prompt you when you open your first Pull Request.

## 2. Coding Standards

All code must adhere to our **High-Integrity C Style Guide** (see `docs/style-guide.md`):

- **No Dynamic Allocation:** Do not use `malloc`, `free`, or `realloc` after initialization
- **MISRA-C Compliance:** Follow MISRA-C:2012 guidelines
- **Explicit Types:** Use `int32_t`, `uint32_t`, not `int` or `long`
- **Deterministic Logic:** No behavior dependent on memory addresses or undefined compiler behavior

## 3. The Definition of Done

A PR is only merged when:

1. ✅ It is linked to a **Requirement ID** in `/docs/requirements/`
2. ✅ It has **100% Branch Coverage** in unit tests
3. ✅ It passes our **Bit-Perfect Test** (identical output on x86 and ARM)
4. ✅ It is **MISRA-C compliant**
5. ✅ It has been reviewed by the Project Lead

## 4. Documentation

Every function must document:
- Purpose
- Preconditions
- Postconditions
- Complexity (O(1), O(n), etc.)
- Determinism guarantee

Example:
```c
/**
 * @brief Multiply two fixed-point numbers.
 * 
 * Precondition: None
 * Postcondition: Returns a * b in Q16.16 format
 * Complexity: O(1) time, O(1) space
 * Determinism: Bit-perfect across all platforms
 */
fixed_t fixed_mul(fixed_t a, fixed_t b);
```

## 5. Getting Started

Look for issues labeled `good-first-issue` or `foundation-layer`.

We recommend starting with:
- Hash table tests
- Fixed-point math operations
- Cross-platform verification

## Questions?

- **Technical questions:** Open an issue
- **General inquiries:** william@fstopify.com
- **Security issues:** Email william@fstopify.com (do not open public issues)

Thank you for helping make deterministic ML inference a reality! 🎯
