# 🧵 GreenThreads: Lightweight User-Level Threading Library

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Language](https://img.shields.io/badge/language-C%2B%2B17-blue)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

**GreenThreads** is a custom implementation of a user-level threading library written in **Modern C++**. It provides a preemptive scheduling environment, allowing developers to spawn, manage, and synchronize threads purely in user space, independent of the kernel's thread management.

This project demonstrates core Operating System concepts including **Context Switching**, **Signal Handling**, **Memory Management**, and **Scheduler Design**.

---

## 🚀 Key Features

* **Preemptive Round-Robin Scheduling:** Implements a time-slicing scheduler using virtual interval timers (`SIGVTALRM`).
* **Low-Level Context Switching:** Direct manipulation of CPU registers (`PC`, `SP`) using `sigsetjmp` and `siglongjmp` to swap execution contexts.
* **Modern C++ Design:** Utilizes **RAII** for automatic resource management (`std::unique_ptr` for thread objects, `std::vector` for stacks) to ensure memory safety.
* **Cross-Platform Architecture:**
    * **Linux (x86_64):** Implements manual pointer mangling to comply with `glibc` security mechanisms.
    * **macOS (ARM64):** Includes a compatibility layer to bypass hardware restrictions for educational simulation.
* **Concurrency Control:** Signal masking mechanisms to create **Critical Sections** and prevent race conditions within the scheduler.

---

## 🛠️ Technical Architecture

### How it Works
Unlike Kernel-level threads (`pthreads`), GreenThreads runs entirely in the user space (the "Many-to-One" model). The library virtualizes the CPU by saving the state of the current function (registers + stack pointer) and restoring the state of another.

### The Scheduling Cycle
1.  **Timer Interrupt:** A virtual timer (`setitimer`) fires a `SIGVTALRM` signal every $X$ microseconds.
2.  **Context Save:** The Signal Handler catches the interrupt. It saves the current thread's CPU state (registers) to its `jmp_buf` environment.
3.  **Scheduling Decision:** The Scheduler picks the next thread from the `READY` queue based on a Round-Robin algorithm.
4.  **Context Restore:** The new thread's state is loaded into the CPU, and execution resumes exactly where it left off.

### Memory Management
Each thread is allocated a dedicated 4KB stack block managed by a `std::vector<char>`. This ensures that when a thread object is destroyed, its stack memory is automatically reclaimed, eliminating common "memory leak" bugs found in legacy C implementations.

---

## 📂 Project Structure

```text
GreenThreads/
├── CMakeLists.txt       # Build configuration
├── Dockerfile           # Linux environment for cross-platform support
├── include/
│   ├── uthreads.h       # Public C-compatible API
│   ├── Scheduler.h      # Scheduler class definition
│   └── Thread.h         # Thread class and context management
├── src/
│   ├── lib.cpp          # API implementation (The "Glue" layer)
│   ├── Scheduler.cpp    # Core scheduling logic & Signal handling
│   └── Thread.cpp       # Stack & Register manipulation
└── examples/
    └── main.cpp         # Demo application
```


## 📦 Installation & Usage

### Prerequisites
* **C++17 Compiler** (GCC/Clang)
* **CMake** (3.10+)
* **Docker** (Highly recommended for macOS / Apple Silicon users)

### 🐳 Running with Docker (Recommended)
Since this library performs low-level stack manipulation specific to CPU architecture, it is best run in a native Linux environment.

**1. Build the Docker Image:**
```bash
docker build -t green-threads .

### 🐳 Running with Docker (Recommended)
Since this library performs low-level stack manipulation specific to CPU architecture, it is best run in a native Linux environment.

**1. Build the Docker Image:**
```bash
docker build -t green-threads .
```

**2. Run the Demo:**
```bash
docker run --rm green-threads
```

### 💻 Local Build (Linux)
If you are running on a native Linux machine:

```bash
mkdir build && cd build
cmake ..
make
./demo_app
```

## 💻 API Example

The library exposes a simple C-compatible API via `uthreads.h`. Here is how you use it:

```cpp
#include "uthreads.h"
#include <iostream>

// Worker function
void heavy_computation() {
    int id = uthread_get_tid();
    std::cout << "Thread " << id << " starting work..." << std::endl;

    // Yield CPU to other threads (Sleep for 2 quantums)
    uthread_sleep(2);

    std::cout << "Thread " << id << " finished." << std::endl;
    uthread_terminate(id);
}

int main() {
    // 1. Initialize library with 100ms time quantum
    if (uthread_init(100000) == -1) {
        std::cerr << "Init failed" << std::endl;
        return 1;
    }

    // 2. Spawn threads
    int t1 = uthread_spawn(heavy_computation);
    int t2 = uthread_spawn(heavy_computation);

    std::cout << "Spawned threads: " << t1 << ", " << t2 << std::endl;

    // 3. Main thread loop (required to keep process alive)
    while(true) {
        // Main thread work...
    }
    return 0;
}
```
## 🧠 Engineering Challenges & Solutions

### 1. The "Pointer Mangling" Challenge
**Problem:** On Linux (x86_64), `glibc` implementation of `setjmp`/`longjmp` encrypts the Stack Pointer (SP) and Program Counter (PC) stored in the buffer for security. Manually overwriting them without encoding causes a segmentation fault.

**Solution:** I implemented a `translate_address` function using inline Assembly (`xor` + `rol` instructions) to correctly mimic the OS's encryption, allowing successful manual context creation.



### 2. Signal Safety & Race Conditions
**Problem:** A timer interrupt occurring exactly while the Scheduler is modifying the `READY` queue leads to undefined behavior and corrupted data structures.

**Solution:** Implemented strictly defined **Critical Sections**. Signals are masked (`sigprocmask`) before any queue operation or context switch and unmasked immediately after, ensuring atomicity.



---

## 📜 License
This project is open-source and available under the MIT License.
