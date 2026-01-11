#include "uthreads.h"
#include <iostream>
#include <unistd.h>

/**
 * @brief Demo Application for GreenThreads
 * This program demonstrates concurrent execution of two user-level threads.
 */

void f1() {
  int i = 0;
  while (1) {
    if (i++ % 1000000 == 0) {
      std::cout << "[Thread 1] Working..." << std::endl;
    }
    // Busy wait to simulate work and allow preemption
    for (int j = 0; j < 1000; j++) {}
  }
}

void f2() {
  int i = 0;
  while (1) {
    if (i++ % 1000000 == 0) {
      std::cout << "   [Thread 2] Working..." << std::endl;
    }
    for (int j = 0; j < 1000; j++) {}
  }
}

int main() {
  std::cout << "--- Starting GreenThreads Demo ---" << std::endl;

  // Initialize with 100ms quantum
  if (uthread_init(100000) == -1) {
    std::cerr << "Initialization failed!" << std::endl;
    return 1;
  }

  std::cout << "Spawning worker threads..." << std::endl;
  int t1 = uthread_spawn(f1);
  int t2 = uthread_spawn(f2);

  std::cout << "Threads created with IDs: " << t1 << ", " << t2 << std::endl;

  // Main thread loop
  while(1) {
    // Main thread stays alive to let others run
  }
  return 0;
}