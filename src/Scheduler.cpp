#include "Scheduler.h"
#include <iostream>
#include <algorithm>

#define MAIN_TID 0

// Global instance reference
Scheduler* g_scheduler = nullptr;

Scheduler::Scheduler(int quantum_usecs) : quantum_usecs(quantum_usecs), current_thread(nullptr) {
  g_scheduler = this;

  // Initialize signal set
  sigemptyset(&vtalrm_set);
  sigaddset(&vtalrm_set, SIGVTALRM);

  // Create the Main Thread (TID 0)
  // We don't provide an entry point because it's already running.
  auto mainThread = std::make_unique<Thread>(MAIN_TID, nullptr);
  mainThread->setState(Thread::State::RUNNING);
  current_thread = mainThread.get();

  threads[MAIN_TID] = std::move(mainThread);
  total_quantums = 1;

  setupTimer();
}

Scheduler::~Scheduler() {
  threads.clear(); // Unique pointers clean up memory automatically
}

void Scheduler::setupTimer() {
  struct sigaction sa = {0};
  sa.sa_handler = &Scheduler::timerHandler;
  if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
    std::cerr << "system error: sigaction failed\n";
    exit(1);
  }
  resetTimer();
}

void Scheduler::resetTimer() {
  timer.it_value.tv_sec = quantum_usecs / 1000000;
  timer.it_value.tv_usec = quantum_usecs % 1000000;
  timer.it_interval.tv_sec = quantum_usecs / 1000000;
  timer.it_interval.tv_usec = quantum_usecs % 1000000;

  if (setitimer(ITIMER_VIRTUAL, &timer, nullptr) < 0) {
    std::cerr << "system error: setitimer failed\n";
    exit(1);
  }
}

void Scheduler::timerHandler(int sig) {
  if (!g_scheduler) return;

  // Enter Critical Section
  g_scheduler->blockSignals();

  // MAGIC HAPPENS HERE: Save the current thread's state.
  // sigsetjmp returns 0 on direct call, and non-zero when returning via longjmp.
  int ret = sigsetjmp(g_scheduler->current_thread->getEnv(), 1);

  if (ret == 0) {
    // State saved. Now switch to the next thread.
    g_scheduler->switchContext();
  } else {
    // We just returned from a longjmp!
    // This means it's now our turn to run again.
    g_scheduler->unblockSignals();
  }
  (void)sig; // Suppress unused warning
}

void Scheduler::switchContext() {
  // 1. Handle the outgoing thread
  if (current_thread->getState() == Thread::State::RUNNING) {
    current_thread->setState(Thread::State::READY);
    ready_queue.push_back(current_thread);
  }

  // 2. Select the next thread (Round Robin)
  if (!ready_queue.empty()) {
    current_thread = ready_queue.front();
    ready_queue.pop_front();
  }

  current_thread->setState(Thread::State::RUNNING);

  // 3. Update stats and reset timer
  total_quantums++;
  current_thread->incrementQuantum();
  resetTimer();

  // 4. Jump to the new thread's saved state
  siglongjmp(current_thread->getEnv(), 1);
}

void Scheduler::blockSignals() {
  sigprocmask(SIG_BLOCK, &vtalrm_set, nullptr);
}

void Scheduler::unblockSignals() {
  sigprocmask(SIG_UNBLOCK, &vtalrm_set, nullptr);
}

int Scheduler::spawn(void (*entryPoint)(void)) {
  blockSignals();
  int tid = generateId();

  auto newThread = std::make_unique<Thread>(tid, entryPoint);
  Thread* ptr = newThread.get();

  threads[tid] = std::move(newThread);
  ready_queue.push_back(ptr);

  unblockSignals();
  return tid;
}

int Scheduler::terminate(int tid) {
  blockSignals();
  if (threads.find(tid) == threads.end()) {
    unblockSignals();
    return -1;
  }

  threads.erase(tid);

  // If we killed ourselves, force a context switch immediately
  if (tid == getCurrentTid()) {
    resetTimer();
    raise(SIGVTALRM);
  }

  unblockSignals();
  return 0;
}

// Placeholder implementations for blocking/sleeping
int Scheduler::block(int tid) { (void)tid; return 0; }
int Scheduler::resume(int tid) { (void)tid; return 0; }
int Scheduler::sleep(int quantums) { (void)quantums; return 0; }

int Scheduler::getCurrentTid() const {
  return current_thread ? current_thread->getId() : -1;
}

int Scheduler::getThreadQuantums(int tid) const {
  if (threads.find(tid) != threads.end()) {
    return threads.at(tid)->getQuantums();
  }
  return -1;
}

int Scheduler::generateId() {
  while (threads.find(next_tid) != threads.end()) {
    next_tid++;
  }
  return next_tid++;
}