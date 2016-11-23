#ifndef LIBS_BASE_OS_H
#define LIBS_BASE_OS_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <cstdint>
#include "system.h"

/**
 * @defgroup osal OS Abstraction layer
 * @{
 */

/**
 * @brief Maximal allowed operation timeout.
 */
#define MAX_DELAY 0xffffffffUL

#ifndef ELAST
// newlib workaround
#if defined _NEWLIB_VERSION && defined __ELASTERROR
#define ELAST __ELASTERROR
#else
#error "stdlib does not define ELAST errno value."
#endif
#endif

/**
 * @brief Enumerator for all possible operating system error codes.
 */
enum OSResult
{
    /** Success */
    OSResultSuccess = 0,

    /** @brief Requested operation is invalid. */
    OSResultInvalidOperation = ELAST,

    /** Requested element was not found. */
    OSResultNotFound = ENOENT,
    /** Interrupted system call */
    OSResultInterrupted = EINTR,
    /** I/O error */
    OSResultIOError = EIO,
    /** Argument list too long */
    OSResultArgListTooLong = E2BIG,
    /** Bad file number */
    OSResultInvalidFileHandle = EBADF,
    /** No children */
    OSResultNoChildren = ECHILD,
    /** Not enough memory */
    OSResultNotEnoughMemory = ENOMEM,
    /** Permission denied */
    OSResultAccessDenied = EACCES,
    /** Bad address */
    OSResultInvalidAddress = EFAULT,
    /** Device or resource busy */
    OSResultBusy = EBUSY,
    /** File exists */
    OSResultFileExists = EEXIST,
    /** Cross-device link */
    OSResultInvalidLink = EXDEV,
    /** No such device */
    OSResultDeviceNotFound = ENODEV,
    /** Not a directory */
    OSResultNotADirectory = ENOTDIR,
    /** Is a directory */
    OSResultIsDirectory = EISDIR,
    /** Invalid argument */
    OSResultInvalidArgument = EINVAL,
    /** Too many open files in system */
    OSResultTooManyOpenFiles = ENFILE,
    /** File descriptor value too large */
    OSResultDescriptorTooLarge = EMFILE,
    /** File too large */
    OSResultFileTooLarge = EFBIG,
    /** No space left on device */
    OSResultOutOfDiskSpace = ENOSPC,
    /** Illegal seek */
    OSResultInvalidSeek = ESPIPE,
    /** Read-only file system */
    OSResultReadOnlyFs = EROFS,
    /** Too many links */
    OSResultTooManyLinks = EMLINK,
    /** Result too large */
    OSResultOutOfRange = ERANGE,
    /** Deadlock */
    OSResultDeadlock = EDEADLK,
    /** No lock */
    OSResultNoLock = ENOLCK,
    /** A non blocking operation could not be immediately completed */
    OSResultWouldBlock = ENODATA,
    /** Operation timed out. */
    OSResultTimeout = ETIME,
    /** Protocol error */
    OSResultProtocolError = EPROTO,
    /** Bad message */
    OSResultInvalidMessage = EBADMSG,
    /** Inappropriate file type or format */
    OSResultInvalidFileFormat = EFTYPE,
    /** Function not implemented */
    OSResultNotImplemented = ENOSYS,
    /** Directory not empty */
    OSResultDirectoryNotEmpty = ENOTEMPTY,
    /** File or path name too long */
    OSResultPathTooLong = ENAMETOOLONG,
    /** Too many symbolic links */
    OSResultLinkCycle = ELOOP,
    /** Operation not supported */
    OSResultNotSupported = EOPNOTSUPP,
    /** Protocol family not supported  */
    OSResultProtocolNotSupported = EPFNOSUPPORT,
    /** No buffer space available */
    OSResultBufferNotAvailable = ENOBUFS,
    /** Protocol not available */
    OSResultProtocolNotAvailable = ENOPROTOOPT,
    /** Unknown protocol */
    OSResultUnknownProtocol = EPROTONOSUPPORT,
    /** Illegal byte sequence */
    OSResultInvalidByteSequence = EILSEQ,
    /** Value too large for defined data type */
    OSResultOverflow = EOVERFLOW,
    /** Operation canceled */
    OSResultCancelled = ECANCELED,

};

/**
 * @brief Macro for verification whether passed OSResult value indicates success.
 */
#define OS_RESULT_SUCCEEDED(x) ((x) == OSResultSuccess)

/**
 * @brief Macro for verification whether passed OSResult value indicates failure.
 */
#define OS_RESULT_FAILED(x) ((x) != OSResultSuccess)

/** @brief Type definition for time span in ms. */
using OSTaskTimeSpan = std::uint32_t;

/** @brief Type definition of handle to system task. */
using OSTaskHandle = void*;

/** @brief Type definition of semaphore handle. */
using OSSemaphoreHandle = void*;

/** @brief Type definition of event group handle. */
using OSEventGroupHandle = void*;

/** @brief Type definition of event group value. */
using OSEventBits = std::uint32_t;

/** @brief Type definition of queue handle */
using OSQueueHandle = void*;

/** @brief Type definition of pulse all handle */
using OSPulseHandle = void*;

/**
 * @brief Pointer to generic system procedure that operates on task.
 *
 * @param[in] task Task handle.
 */
using OSTaskProcedure = void (*)(OSTaskHandle task);

/**
 * @brief Definition of operating system interface.
 */
class System
{
  public:
    /**
     * @brief Creates new task
     *
     * @param[in] entryPoint Pointer to task procedure.
     * @param[in] taskName Name of the new task.
     * @param[in] stackSize Size of the new task's stack in words.
     * @param[in] taskParameter Pointer to caller supplied object task context.
     * @param[in] priority New task priority.
     * @param[out] taskHandle Pointer to variable that will be filled with the created task handle.
     * @return Operation status.
     */
    static OSResult CreateTask(OSTaskProcedure entryPoint,
        const char* taskName,
        std::uint16_t stackSize,
        void* taskParameter,
        std::uint32_t priority,
        OSTaskHandle* taskHandle);

    template <typename Param>
    static OSResult CreateTask(void (*entryPoint)(Param&),
        Param& param,
        const char* taskName,
        std::uint16_t stackSize,
        std::uint32_t priority,
        OSTaskHandle* taskHandle = nullptr);

    /**
     * @brief Suspends current task execution for specified time period.
     * @param[in] time Time period in ms.
     */
    static void SleepTask(const OSTaskTimeSpan time);

    /**
     * @brief Resumes execution of requested task.
     *
     * @param[in] task Task handle.
     * @remark This procedure should not be used from within interrupt service routine.
     */
    static void ResumeTask(OSTaskHandle task);

    /**
     * @brief Suspend execution of requested task.
     *
     * @param[in] task Task handle.
     * @remark This procedure should not be used from within interrupt service routine.
     */
    static void SuspendTask(OSTaskHandle task);

    /**
     * @brief Runs the system scheduler.
     */
    static void RunScheduler();

    /**
     * @brief Creates binary semaphore.
     *
     */
    static OSSemaphoreHandle CreateBinarySemaphore();

    /**
     * @brief Acquires semaphore.
     *
     * @param[in] semaphore Handle to the semaphore that should be acquired.
     * @param[in] timeout Operation timeout.
     * @return Operation status.
     * @remark This procedure should not be used from within interrupt service routine.
     */
    static OSResult TakeSemaphore(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout);

    /**
     * @brief Releases semaphore.
     *
     * @param[in] semaphore Handle to semaphore that should be released.
     * @return Operation status.
     * @remark This procedure should not be used from within interrupt service routine.
     */
    static OSResult GiveSemaphore(OSSemaphoreHandle semaphore);

    /**
     * @brief Creates event group object.
     *
     * @return Event group handle on success, NULL otherwise.
     */
    static OSEventGroupHandle CreateEventGroup();

    /**
     * @brief Sets specific bits in the event group.
     *
     * @param[in] eventGroup Handle to the event group that should be updated.
     * @param[in] bitsToChange Bits that should be set.
     * @returns The value of the event group at the time the call to xEventGroupSetBits() returns.
     *
     * There are two reasons why the returned value might have the bits specified by the bitsToChange
     * parameter cleared:
     *  - If setting a bit results in a task that was waiting for the bit leaving the blocked state
     *  then it is possible the bit will have been cleared automatically.
     *  - Any unblocked (or otherwise Ready state) task that has a priority above that of the task
     *  that called EventGroupSetBits() will execute and may change the event group value before
     *  the call to EventGroupSetBits() returns.
     * @remark This procedure should not be used from within interrupt service routine.
     */
    static OSEventBits EventGroupSetBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange);

    /**
     * @brief Clears specific bits in the event group.
     *
     * @param[in] eventGroup The affected event group handle.
     * @param[in] bitsToWaitFor Bits that the caller is interested in.
     * @param[in] waitAll Flat indicating whether this procedure should return when all requested bits are set.
     * @param[in] autoReset Flag indicating whether the signaled bits should be cleared on function return.
     * @param[in] timeout Operation timeout in ms.
     */
    static OSEventBits EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange);

    /**
     * @brief Suspends current task execution until the
     * specific bits in the event group are set.
     *
     * @see OSEventGroupWaitForBits for the details.
     */
    static OSEventBits EventGroupWaitForBits(
        OSEventGroupHandle eventGroup, const OSEventBits bitsToWaitFor, bool waitAll, bool autoReset, const OSTaskTimeSpan timeout);

    /**
     * @brief Allocates block of memory from OS heap
     *
     * @param[in] size Size of the block to alloc
     */
    static void* Alloc(std::size_t size);

    /**
     * @brief Frees block of memory
     * @param[in] ptr Pointer to block to free
     * @see OSFree
     */
    static void Free(void* ptr);

    /**
     * @brief Creates queue
     *
     * @param[in] maxQueueElements Maximum number of elements in queue
     * @param[in] elementSize Size of single element
     * @return Queue handle on success, NULL otherwise
     */
    static OSQueueHandle CreateQueue(std::size_t maxQueueElements, std::size_t elementSize);

    /**
     * @brief Receives element form queue
     *
     * @param[in] queue Queue handle
     * @param[out] element Buffer for element
     * @param[in] timeout Operation timeout in ms.
     * @return TRUE if element was received, FALSE on timeout
     */
    static bool QueueReceive(OSQueueHandle queue, void* element, OSTaskTimeSpan timeout);

    /**
     * @brief Receives element form queue in interrupt handler
     *
     * @param[in] queue Queue handle
     * @param[out] element Buffer for element
     * @return TRUE if element was received, FALSE on timeout
     */
    static bool QueueReceiveFromISR(OSQueueHandle queue, void* element);

    /**
     * @brief Sends element to queue
     *
     * @param[in] queue Queue handle
     * @param[in] element Element to send to queue
     * @param[in] timeout Operation timeout in ms
     * @return TRUE if element was received, FALSE on timeout
     */
    static bool QueueSend(OSQueueHandle queue, void* element, OSTaskTimeSpan timeout);

    /**
     * @brief Sends element to queue in interrupt handler
     *
     * @param[in] queue Queue handle
     * @param[in] element Element to send to queue
     * @return TRUE if element was received, FALSE on timeout
     */
    static bool QueueSendISR(OSQueueHandle queue, void* element);

    /**
     * @brief Overwrites element in queue
     *
     * @param[in] queue QueueHandle
     * @param[in] element Element to send to queue
     */
    static void QueueOverwrite(OSQueueHandle queue, const void* element);

    /**
     * @brief Procedure that should be called at the end of interrupt handler
     *
     */
    static void EndSwitchingISR();

    /**
     * @brief Creates pulse all event
     *
     */
    static OSPulseHandle CreatePulseAll();

    /**
     * @brief Waits for pulse
     *
     * @param[in] handle Pulse handle
     * @param[in] timeout Operation timeout in ms
     * @return Wait result
     */
    static OSResult PulseWait(OSPulseHandle handle, OSTaskTimeSpan timeout);

    /**
     * @brief Sets pulse event
     *
     * @param[in] handle Pulse handle
     */
    static void PulseSet(OSPulseHandle handle);
};

template <typename Param>
OSResult System::CreateTask(void (*entryPoint)(Param&),
    Param& param,
    const char* taskName,
    std::uint16_t stackSize,
    std::uint32_t priority,
    OSTaskHandle* taskHandle)
{
    auto entryPointPtr = reinterpret_cast<OSTaskProcedure>(entryPoint);

    return System::CreateTask(entryPointPtr, taskName, stackSize, &param, priority, taskHandle);
}

/** @}*/

#endif
