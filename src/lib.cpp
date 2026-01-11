#include "uthreads.h"
#include "Scheduler.h"
#include <iostream>

// Internal static instance of the scheduler
static Scheduler* scheduler = nullptr;

int uthread_init(int quantum_usecs) {
  if (quantum_usecs <= 0) return -1;
  if (scheduler) return -1; // Already initialized

  try {
    scheduler = new Scheduler(quantum_usecs);
  } catch (...) {
    return -1;
  }
  return 0;
}

int uthread_spawn(void (*entry_point)(void)) {
  if (!scheduler || !entry_point) return -1;
  return scheduler->spawn(entry_point);
}

int uthread_terminate(int tid) {
  if (!scheduler) return -1;
  if (tid == 0) {
    // Terminating the main thread kills the process
    delete scheduler;
    scheduler = nullptr;
    exit(0);
  }
  return scheduler->terminate(tid);
}

// Forwarding calls to the scheduler
int uthread_block(int tid) { return scheduler ? scheduler->block(tid) : -1; }
int uthread_resume(int tid) { return scheduler ? scheduler->resume(tid) : -1; }
int uthread_sleep(int q) { return scheduler ? scheduler->sleep(q) : -1; }
int uthread_get_tid() { return scheduler ? scheduler->getCurrentTid() : -1; }
int uthread_get_total_quantums() { return scheduler ? scheduler->getTotalQuantums() : -1; }
int uthread_get_quantums(int tid) { return scheduler ? scheduler->getThreadQuantums(tid) : -1; }