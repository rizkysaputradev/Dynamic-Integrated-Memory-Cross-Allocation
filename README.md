# Dynamic Integrated Memory Cross Allocation (DIMCA)

DIMCA (Dynamic Integrated Memory Cross Allocation) is a custom algorithm designed to enhance memory allocation performance by optimizing how data capsules are distributed across multi-level memory pools. The goal is to minimize allocation time penalties due to mismatches between data and memory levels, simulating scenarios found in hierarchical memory systems (e.g., CPU cache levels, RAM tiers, or storage stacks).

---

## 📚 Theoretical Motivation

Modern computing systems operate with tiered memory hierarchies:
- **Level 1 (L1)**: Fastest but smallest (e.g., CPU cache)
- **Level 2 (L2)**: Larger but slower (e.g., main memory)
- **Level 3 (L3)**: Largest but slowest (e.g., disk or virtual memory)

### Problem

Traditional allocation models often place data indiscriminately, causing performance drops due to:
- **Time penalties** when accessing deeper memory levels
- **Lack of alignment** between data's urgency or criticality and memory proximity

DIMCA seeks to fix this by introducing an adaptive and dynamic placement model that:
- Matches **data capsule priority levels** to **memory level tiers**
- Penalizes mismatches in levels with a cost function
- Supports **concurrent allocation** across multiple threads

---

## 🧠 Core Concepts

### Data Capsule Structure
```c
typedef struct {
    int id;         // Data capsule ID
    int value;      // Relevance or priority (higher = more urgent)
    int level;      // Preferred memory level
} DataCapsule;
```

## 🔁 Allocation Cost Formula
DIMCA penalizes level mismatches between data and memory using a linear penalty function:
```pgsql
Time Cost = 1.0 + α × |L_d - L_m|
```

Where:
- **L_d** = data capsule's preferred level  
- **L_m** = actual memory level where it was allocated  
- **α** = mismatch penalty factor (default = `0.5`)

### 📌 Example:
If a data capsule prefers Level 1 but is placed in Level 3:
```pgsql
Time Cost = 1.0 + 0.5 × |1 - 3| = 2.0
```

This encourages level-aligned allocation while still permitting fallback if required.

---

## ⚙️ How DIMCA Works

### 1. Memory Pool Initialization
Memory is initialized with configurable sizes and levels:
```c
int mem_sizes[] = {3, 3, 3};       // 3 slots per level
int mem_levels[] = {1, 2, 3};      // Level identifiers
```

A global memory map is flattened to support linear scanning and allocation.

### 2. Data Capsule Batching
Capsules are grouped into batches, and each batch simulates one round of allocation steps.

### 3. Data Capsule Batching
In general mode, DIMCA uses multi-threading to perform allocations in parallel:
- Attempts to allocate in preferred level
- Falls back to other levels if needed
- Calculates and accumulates time cost for each allocation

--

## 🔧 File Structure
```makefile
Dynamic-Integrated-Memory-Cross-Allocation/
├── include/
│   └── dimca_general.h          # General header file
├── src/
│   ├── dimca.c                  # Shared utilities
│   ├── dimca_basic.c           # Basic (sequential) version
│   ├── dimca_general.c         # General (parallel) version
├── tests/
│   ├── main.c                  # Legacy test
│   ├── main_basic.c           # Runner for basic mode
│   ├── main_general.c         # Runner for general mode
├── Makefile
└── README.md                   # This file
```

---

## 🚀 Running the Simulation

### ▶️ Run the Simulator
```bash
make run_basic
make run_general
```
Further adjustment will be made for future developments

---

## 🔬 Future Development Ideas
- ✅ LRU-based eviction for full memory levels
- ✅ Real-time simulation interface / GUI
- ✅ Workload-aware capsule ranking
- ✅ Allocation visualization via heatmaps or memory charts

---

## 👤 Author
Developed by **Rizky Saputra**
As part of an independent Hardware and Memory Systems Research Project (2025)
