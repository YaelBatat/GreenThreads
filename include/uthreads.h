#pragma once

/**
 * @file uthreads.h
 * @brief Public API for the GreenThreads User-Level Thread Library.
 * * This library provides a simple interface for creating and managing user-level threads
 * with Round-Robin scheduling. It is designed to demonstrate low-level OS concepts
 * such as context switching, signal handling, and memory management.
 * * @author Yael Batat
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the thread library.
 * * Must be called once before any other function. Sets up the internal scheduler
 * and the main thread.
 * * @param quantum_usecs Duration of a single time slice in microseconds.
 * @return 0 on success, -1 on failure.
 */
int uthread_init(int quantum_usecs);

/**
 * @brief Creates a new thread.
 * * The new thread is added to the READY queue and will run according to the
 * scheduling policy.
 * * @param entry_point Pointer to the function to execute.
 * @return The ID of the created thread (tid), or -1 on failure.
 */
int uthread_spawn(void (*entry_point)(void));

/**
 * @brief Terminates a specific thread.
 * * Frees all resources associated with the thread. If the main thread (tid=0)
 * is terminated, the entire process exits.
 * * @param tid The ID of the thread to terminate.
 * @return 0 on success, -1 on failure.
 */
int uthread_terminate(int tid);

/**
 * @brief Blocks a thread.
 * * Moves the thread to the BLOCKED state. It will not run again until resumed.
 * * @param tid The ID of the thread to block.
 * @return 0 on success, -1 on failure.
 */
int uthread_block(int tid);

/**
 * @brief Resumes a blocked thread.
 * * Moves the thread from BLOCKED to READY state.
 * * @param tid The ID of the thread to resume.
 * @return 0 on success, -1 on failure.
 */
int uthread_resume(int tid);

/**
 * @brief Puts the calling thread to sleep.
 * * The thread will yield the CPU and return to the READY queue after the
 * specified number of quantums.
 * * @param num_quantums Number of quantums to sleep.
 * @return 0 on success, -1 on failure.
 */
int uthread_sleep(int num_quantums);

/**
 * @brief Returns the ID of the currently running thread.
 * @return Thread ID (non-negative integer).
 */
int uthread_get_tid();

/**
 * @brief Returns the total number of quantums elapsed since library initialization.
 * @return Total quantum count.
 */
int uthread_get_total_quantums();

/**
 * @brief Returns the number of quantums a specific thread has run.
 * @param tid The thread ID.
 * @return Quantum count for the thread, or -1 if tid is invalid.
 */
int uthread_get_quantums(int tid);

#ifdef __cplusplus
}
#endif