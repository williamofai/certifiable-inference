# certifiable-inference

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/williamofai/certifiable-inference)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)
[![MISRA Compliance](https://img.shields.io/badge/MISRA--C-2012-blue)](docs/misra-compliance.md)

**Deterministic, bit-perfect AI inference for safety-critical systems.**

Pure C99. Zero dynamic allocation. Certifiable for DO-178C, IEC 62304, and ISO 26262.

---

## The Problem

Most AI infrastructure is built for research, where "mostly reproducible" is good enough. In aerospace, medical devices, and autonomous systems, non-determinism isn't just a bug—it's a **liability**.

Modern ML inference engines (TensorFlow Lite, ONNX Runtime, PyTorch Mobile) are non-deterministic:
- Floating-point operations vary across platforms
- Hash table iteration order is unpredictable  
- Memory allocation affects behavior

For safety-critical systems, you cannot prove correctness if behavior varies.

**Read more:** [Why Floating Point Is Dangerous](https://speytech.com/ai-architecture/floating-point-danger/)

## The Solution

`certifiable-inference` replaces the "black box" of modern ML with a deterministic pipeline built on three pillars:

### 1. Exact Math
Fixed-point arithmetic ensures `Input A + Model B = Output C` across all platforms (x86, ARM, RISC-V), forever. No floating-point drift.

### 2. Bounded Resources  
No `malloc()` after initialization. O(1) memory and time complexity for every inference pass. Predictable behavior for real-time systems.

### 3. Traceable Logic
Pure C99 implementation with zero hidden dependencies. Designed for MISRA-C compliance and formal verification.

**Result:** Same input → Same output. Always. On any platform. Forever.

## Status

🚧 **In Development** - Foundation being built

**Current components:**
- ✅ Deterministic hash table (complete)
- 🚧 Fixed-point math library (in progress)
- 📋 Neural network layers (planned)
- 📋 ONNX model loader (planned)

## Quick Start

```c
#include "deterministic_hash.h"

// Initialize table with fixed memory buffer
uint8_t buffer[1024];
d_table_t map;
d_table_init(&map, buffer, sizeof(buffer));

// Iteration order guaranteed by key-sort, not memory address
d_table_insert(&map, "cardiac_rate", 72);
d_table_insert(&map, "oxygen_sat", 98);

// Iterate - always same order
d_table_iterate(&map, print_callback);
```

## Contributing

We welcome contributions from senior systems engineers! See [CONTRIBUTING.md](CONTRIBUTING.md).

**Important:** All contributors must sign a [Contributor License Agreement](CONTRIBUTOR-LICENSE-AGREEMENT.md).

## License

**Dual Licensed:**
- **Open Source:** GNU General Public License v3.0 (GPLv3)
- **Commercial:** Contact william@fstopify.com

## Contact

**William Murray**  
william@fstopify.com  
[speytech.com](https://speytech.com)

---

Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
