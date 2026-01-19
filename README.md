# certifiable-inference

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/williamofai/certifiable-inference)
[![Tests](https://img.shields.io/badge/tests-64%20passing-brightgreen)](https://github.com/williamofai/certifiable-inference)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue)](LICENSE)
[![MISRA Compliance](https://img.shields.io/badge/MISRA--C-2012-blue)](docs/misra-compliance.md)

**Deterministic, bit-perfect AI inference for safety-critical systems.**

Pure C99. Zero dynamic allocation. Certifiable for DO-178C, IEC 62304, and ISO 26262.

🔴 **Live Demo:** [inference.speytech.com](https://inference.speytech.com)

---

## The Problem

Most AI infrastructure is built for research, where "mostly reproducible" is good enough. In aerospace, medical devices, and autonomous systems, non-determinism isn't just a bug—it's a **liability**.

Modern ML inference engines (TensorFlow Lite, ONNX Runtime, PyTorch Mobile) are non-deterministic:

* Floating-point operations vary across platforms
* Hash table iteration order is unpredictable
* Memory allocation affects behavior

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

**Current components:**

* ✅ Fixed-point arithmetic (Q16.16, deterministic across platforms)
* ✅ Matrix operations (multiply, transpose, element-wise)
* ✅ 2D Convolution (zero dynamic allocation, O(OH×OW×KH×KW))
* ✅ Activation functions (ReLU, deterministic thresholding)
* ✅ Max Pooling (2×2 stride-2, dimension reduction)
* ✅ Timing verification (proven <5% jitter for 95th percentile)
* 📋 Model loader (ONNX import - planned)
* 📋 Quantization tools (FP32→Q16.16 conversion - planned)

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make
make test
```

### Basic Inference Pipeline

```c
#include "matrix.h"
#include "convolution.h"
#include "activation.h"
#include "pooling.h"

// Pre-allocated buffers (no malloc)
fixed_t input_buf[256];    // 16×16 input
fixed_t kernel_buf[9];     // 3×3 kernel
fixed_t conv_buf[196];     // 14×14 after conv
fixed_t pool_buf[49];      // 7×7 after pooling

// Initialize matrices
fx_matrix_t input, kernel, conv_out, pool_out;
fx_matrix_init(&input, input_buf, 16, 16);
fx_matrix_init(&kernel, kernel_buf, 3, 3);
fx_matrix_init(&conv_out, conv_buf, 14, 14);
fx_matrix_init(&pool_out, pool_buf, 7, 7);

// Load input and kernel (quantized to fixed-point)
load_input(&input);
load_kernel(&kernel);

// Inference pipeline
fx_conv2d(&input, &kernel, &conv_out);    // Convolution
fx_relu(&conv_out, &conv_out);            // Activation
fx_maxpool_2x2(&conv_out, &pool_out);     // Dimension reduction

// Result: 7×7 feature map, bit-perfect across all platforms
```

## Architecture

### Fixed-Point Arithmetic (Q16.16)

* **16 integer bits** + **16 fractional bits**
* Range: -32768.0 to +32767.99998
* Resolution: 0.0000152588 (1/65536)
* **Bit-perfect** across x86, ARM, RISC-V, MIPS

### Zero Dynamic Allocation

* All buffers pre-allocated by caller
* Stack usage: O(1) for all operations
* No `malloc()`, `free()`, or heap fragmentation
* Enables static memory analysis (required for DO-178C Level A)

### Deterministic Execution

* Fixed iteration counts (dimension-dependent, not data-dependent)
* No floating-point operations
* No data-dependent branches in hot paths
* **Proven <5% jitter** at 95th percentile (see `tests/benchmarks/`)

## Testing

### Unit Tests

```bash
cd build
./test_fixed_point    # Fixed-point arithmetic
./test_matrix         # Matrix operations
./test_convolution    # 2D convolution
./test_pooling        # Max pooling
```

### Benchmarks

```bash
./timing_benchmark    # Execution time consistency
```

Expected results:

* **Conv2D (16×16→14×14):** ~13-14μs, <5% P95 jitter
* **MatMul (10×10×10×10):** ~6μs, <5% P95 jitter

## Documentation

Complete requirements traceability maintained in `docs/requirements/`:

* **SRS-001:** Matrix Operations
* **SRS-002:** Fixed-Point Arithmetic
* **SRS-003:** Memory Management
* **SRS-004:** Convolution
* **SRS-005:** Activation Functions
* **SRS-006:** Numerical Stability
* **SRS-007:** Deterministic Execution Timing
* **SRS-008:** Max Pooling

Each requirement document includes mathematical specifications, compliance mappings, verification methods, and traceability to code and tests.

## Related Projects

| Project | Description |
|---------|-------------|
| [certifiable-data](https://github.com/williamofai/certifiable-data) | Deterministic data pipeline |
| [certifiable-training](https://github.com/williamofai/certifiable-training) | Deterministic training engine |
| [certifiable-quant](https://github.com/williamofai/certifiable-quant) | Deterministic quantization |
| [certifiable-deploy](https://github.com/williamofai/certifiable-deploy) | Deterministic model packaging |
| [certifiable-inference](https://github.com/williamofai/certifiable-inference) | Deterministic inference engine |

Together, these projects provide a complete deterministic ML pipeline for safety-critical systems:
```
certifiable-data → certifiable-training → certifiable-quant → certifiable-deploy → certifiable-inference
```
## Why This Matters

### Medical Devices

A pacemaker must deliver a signal within a **10ms window**. Non-deterministic timing causes life-threatening delays.

### Autonomous Vehicles

ISO 26262 ASIL-D requires **provable worst-case execution time**. Floating-point variance makes this impossible.

### Aerospace

DO-178C Level A demands **complete requirements traceability**. "Black box" ML cannot be certified.

This engine provides the foundation for AI in systems where lives depend on the answer.

## Compliance Support

This implementation is designed to support certification under:

* **DO-178C** (Aerospace software)
* **IEC 62304** (Medical device software)
* **ISO 26262** (Automotive functional safety)
* **IEC 61508** (Industrial safety systems)

Documentation includes Requirements Traceability Matrix (RTM), Software Requirements Specifications (SRS), Verification & Validation reports, and WCET analysis methodology.

For compliance packages and hardware porting assistance, contact below.

## Deep Dives

Want to understand the engineering principles behind certifiable-inference?

**Fixed-Point & Determinism:**
- [Fixed-Point Neural Networks: The Math Behind Q16.16](https://speytech.com/insights/fixed-point-neural-networks/)
- [Bit-Perfect Reproducibility: Why It Matters and How to Prove It](https://speytech.com/insights/bit-perfect-reproducibility/)
- [From Proofs to Code: Mathematical Transcription in C](https://speytech.com/insights/mathematical-proofs-to-code/)

**Real-Time & Safety:**
- [WCET for Neural Network Inference](https://speytech.com/ai-architecture/wcet-neural-network-inference/)
- [The Real Cost of Dynamic Memory in Safety-Critical Systems](https://speytech.com/insights/dynamic-memory-safety-critical/)

**Certification Standards:**
- [DO-178C Level A Certification: How Deterministic Execution Can Streamline Certification Effort](https://speytech.com/insights/do178c-certification/)
- [IEC 62304 Class C: What Medical Device Software Actually Requires](https://speytech.com/insights/iec-62304-class-c/)
- [ISO 26262 and ASIL-D: The Role of Determinism](https://speytech.com/insights/iso-26262-asil-d-determinism/)

**Production ML Architecture:**
- [A Complete Deterministic ML Pipeline for Safety-Critical Systems](https://speytech.com/ai-architecture/deterministic-ml-pipeline/)
- [TFLite and DO-178C: The Certification Gap](https://speytech.com/ai-architecture/tflite-do178c-challenges/)

## Contributing

We welcome contributions from systems engineers working in safety-critical domains. See [CONTRIBUTING.md](CONTRIBUTING.md).

**Important:** All contributors must sign a [Contributor License Agreement](CONTRIBUTOR-LICENSE-AGREEMENT.md).

## License

**Dual Licensed:**

* **Open Source:** GNU General Public License v3.0 (GPLv3)
* **Commercial:** Available for proprietary use in safety-critical systems

For commercial licensing and compliance documentation packages, contact below.

## Patent Protection

This implementation is built on the **Murray Deterministic Computing Platform (MDCP)**,
protected by UK Patent **GB2521625.0**.

For commercial licensing inquiries: william@fstopify.com

## About

Built by **SpeyTech** in the Scottish Highlands.

30 years of UNIX infrastructure experience applied to deterministic computing for safety-critical systems.

Patent: UK GB2521625.0 - Murray Deterministic Computing Platform (MDCP)

**Contact:**  
William Murray  
william@fstopify.com  
[speytech.com](https://speytech.com)

---

*Building deterministic AI systems for when lives depend on the answer.*

Copyright © 2026 The Murray Family Innovation Trust. All rights reserved.
