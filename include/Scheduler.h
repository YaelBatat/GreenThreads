#pragma once
#include "Thread.h"
#include <deque>
#include <unordered_map>
#include <memory>
#include <sys/time.h>
#include <signal.h>

/**
 * @class Scheduler
 * @brief Manages thread scheduling using a Round-Robin algorithm.
 * * Handles the ready queue, timer signals (SIGVTALRM), and context switching logic.
 * Implements a Singleton-like pattern for signal handling access.
 */
class Scheduler {
 public:
  Scheduler(int quantum_usecs);
  ~Scheduler();

  // Core functionality
  int spawn(void (*entryPoint)(void));
  int terminate(int tid);
  int block(int tid);
  int resume(int tid);
  int sleep(int quantums);

  // State queries
  int getCurrentTid() const;
  int getTotalQuantums() const { return total_quantums; }
  int getThreadQuantums(int tid) const;

  /**
   * @brief Signal handler for SIGVTALRM.
   * Must be static to be compatible with sigaction.
   */
  static void timerHandler(int sig);

  /**
   * @brief Performs the actual Context Switch.
   * Saves the current thread's state (via sigsetjmp) and jumps to the next one.
   */
  void switchContext();

 private:
  // Ownership of threads is managed via unique_ptr
  std::unordered_map<int, std::unique_ptr<Thread>> threads;

  // Round-Robin Queue (non-owning pointers)
  std::deque<Thread*> ready_queue;

  Thread* current_thread;

  int next_tid = 1; // 0 is reserved for Main
  int total_quantums = 0;
  int quantum_usecs;

  // Timer resources
  struct itimerval timer;
  sigset_t vtalrm_set;

  void setupTimer();
  void resetTimer();

  // Critical section management
  void blockSignals();
  void unblockSignals();
  int generateId();
};

// Global pointer for the signal handler
extern Scheduler* g_scheduler;