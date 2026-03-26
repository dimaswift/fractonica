#ifndef FRACTONICA_ITASK_H
#define FRACTONICA_ITASK_H

#include <stddef.h>
#include <stdint.h>

namespace Fractonica {

    /**
     * CPU core assignment for tasks
     */
    enum class CpuCore {
        Core0 = 0,
        Core1 = 1,
        Any = -1    // Let scheduler decide
    };

    /**
     * Task configuration
     */
    struct TaskConfig {
        const char* name;       // Task name for debugging
        uint32_t stackSize;     // Stack size in bytes
        uint8_t priority;       // Task priority (higher = more priority)
        CpuCore core;           // CPU core to pin task to
    };

    /**
     * Abstract interface for a runnable task
     * Implement this to create tasks that can run on different cores
     */
    class IRunnable {
    public:
        virtual ~IRunnable() = default;

        /**
         * Main task function - called repeatedly while task is running
         * Implementation should handle its own loop logic
         */
        virtual void run() = 0;

        /**
         * Called once when task starts, before run() loop begins
         * Override to perform initialization
         */
        virtual void onStart() {}

        /**
         * Called once when task is stopping
         * Override to perform cleanup
         */
        virtual void onStop() {}
    };

    /**
     * Abstract interface for task handle
     * Represents a running task and provides control over it
     */
    class ITaskHandle {
    public:
        virtual ~ITaskHandle() = default;

        /**
         * Check if task is currently running
         */
        virtual bool isRunning() const = 0;

        /**
         * Request task to stop
         * Non-blocking - task may take time to actually stop
         */
        virtual void requestStop() = 0;

        /**
         * Wait for task to complete
         * @param timeoutMs Maximum time to wait in milliseconds, 0 = forever
         * @return true if task stopped, false if timeout
         */
        virtual bool waitForStop(uint32_t timeoutMs = 0) = 0;

        /**
         * Send a notification to wake up the task
         * @param value Notification value to send
         */
        virtual void notify(uint32_t value = 1) = 0;

        /**
         * Get remaining stack space in bytes (for debugging)
         */
        virtual uint32_t getStackHighWaterMark() const = 0;
    };

    /**
     * Abstract interface for task scheduler/manager
     * Creates and manages tasks across CPU cores
     */
    class ITaskScheduler {
    public:
        virtual ~ITaskScheduler() = default;

        /**
         * Create and start a new task
         * @param runnable The runnable to execute (caller retains ownership)
         * @param config Task configuration
         * @return Task handle, or nullptr on failure. Caller owns the returned pointer.
         */
        virtual ITaskHandle* createTask(IRunnable* runnable, const TaskConfig& config) = 0;

        /**
         * Get current CPU core ID
         */
        virtual int getCurrentCore() const = 0;

        /**
         * Delay current task for specified milliseconds
         */
        virtual void delay(uint32_t ms) = 0;

        /**
         * Get current time in milliseconds since boot
         */
        virtual uint32_t millis() const = 0;

        /**
         * Yield to other tasks
         */
        virtual void yield() = 0;
    };

    /**
     * Abstract interface for mutex/semaphore
     * Used for synchronization between tasks
     */
    class IMutex {
    public:
        virtual ~IMutex() = default;

        /**
         * Acquire the mutex
         * @param timeoutMs Maximum time to wait, 0 = forever
         * @return true if acquired, false if timeout
         */
        virtual bool lock(uint32_t timeoutMs = 0) = 0;

        /**
         * Release the mutex
         */
        virtual void unlock() = 0;

        /**
         * Try to acquire without waiting
         * @return true if acquired
         */
        virtual bool tryLock() = 0;
    };

    /**
     * Factory for creating synchronization primitives
     */
    class ISyncFactory {
    public:
        virtual ~ISyncFactory() = default;

        /**
         * Create a new mutex
         * @return Mutex instance. Caller owns the returned pointer.
         */
        virtual IMutex* createMutex() = 0;
    };

} // namespace Fractonica

#endif // FRACTONICA_ITASK_H
