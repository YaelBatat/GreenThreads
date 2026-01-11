#include "Thread.h"
#include <signal.h>

#define STACK_SIZE 4096

Thread::Thread(int id, EntryPoint entryPoint)
    : id(id), state(State::READY), entryPoint(entryPoint),
      stack(STACK_SIZE), run_quantums(0), sleep_quantum(0)
{
  setupContext();
}

void Thread::sleep(int quantums) {
  sleep_quantum = quantums;
}

void Thread::tickSleep() {
  if (sleep_quantum > 0) {
    sleep_quantum--;
  }
}

address_t Thread::translate_address(address_t addr) {
#ifdef __linux__
  // On Linux x86_64, glibc mangles the pointers in jmp_buf.
    // We must mimic this behavior to manually set SP/PC.
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
#else
  // On macOS (ARM64/x86) and other systems, this mangling is not used
  // or works differently. We return the address as-is to allow compilation.
  return addr;
#endif
}

void Thread::setupContext() {
  // 1. Initialize the buffer with the current context
  sigsetjmp(env, 1);

  // 2. Calculate the correct Stack Pointer (SP)
  // Point to the top of the allocated stack memory
  address_t sp = (address_t)stack.data() + STACK_SIZE - sizeof(address_t);

  // 3. Get the Program Counter (PC) - entry point address
  address_t pc = (address_t) *entryPoint.target<void(*)()>();

  // 4. Manually override SP and PC in the jump buffer
#if defined(__linux__) && defined(__x86_64__)
  (env->__jmpbuf)[JB_SP] = translate_address(sp);
    (env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&env->__saved_mask);

#elif defined(__APPLE__)
  // macOS compatibility bypass (Warning: Runtime behavior depends on arch)
  ((long*)env)[JB_SP] = translate_address(sp);
  ((long*)env)[JB_PC] = translate_address(pc);
#else
  // Fallback
    ((long*)env)[JB_SP] = translate_address(sp);
    ((long*)env)[JB_PC] = translate_address(pc);
#endif
}