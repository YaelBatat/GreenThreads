#pragma once
#include <vector>
#include <functional>
#include <csetjmp>
#include <cstdint> // For uintptr_t

// Define address type for portability
typedef uintptr_t address_t;

// Context buffer indices for x86_64 architecture (Linux)
#define JB_SP 6
#define JB_PC 7

/**
 * @class Thread
 * @brief Represents a single execution unit (User-Level Thread).
 * * Manages the thread's stack, execution context (CPU registers), and state.
 * Utilizes RAII for memory management (std::vector for stack).
 */
class Thread {
 public:
  enum class State {
      READY,
      RUNNING,
      BLOCKED,
      TERMINATED
  };

  // Using std::function allows for flexible entry points (including lambdas)
  using EntryPoint = std::function<void()>;

  Thread(int id, EntryPoint entryPoint);
  ~Thread() = default; // Stack is automatically freed by std::vector

  int getId() const { return id; }
  State getState() const { return state; }
  void setState(State newState) { state = newState; }

  // Quantum accounting
  void incrementQuantum() { run_quantums++; }
  int getQuantums() const { return run_quantums; }

  // Sleep management
  void sleep(int quantums);
  void tickSleep();
  bool isAwake() const { return sleep_quantum <= 0; }

  // Access to the jump buffer for context switching
  sigjmp_buf& getEnv() { return env; }

 private:
  int id;
  State state;
  EntryPoint entryPoint;

  // Each thread gets its own stack memory block (4KB)
  std::vector<char> stack;

  // Buffer to save CPU state (Registers, PC, SP)
  sigjmp_buf env;

  int run_quantums;
  int sleep_quantum;

  /**
   * @brief Prepares the initial context for the thread.
   * Calculates the Stack Pointer (SP) and Program Counter (PC).
   */
  void setupContext();

  /**
   * @brief Handles pointer mangling (security feature in glibc).
   * Necessary for x86_64 Linux compatibility.
   */
  static address_t translate_address(address_t addr);
};