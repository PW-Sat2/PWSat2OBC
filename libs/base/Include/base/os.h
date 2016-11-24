#ifndef LIBS_BASE_OS_H
#define LIBS_BASE_OS_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <cstdint>
#include <type_traits>
#include "system.h"
#include "utils.h"

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
enum class OSResult
{
    /** Success */
    Success = 0,

    /** @brief Requested operation is invalid. */
    InvalidOperation = ELAST,

    /** Requested element was not found. */
    NotFound = ENOENT,
    /** Interrupted system call */
    Interrupted = EINTR,
    /** I/O error */
    IOError = EIO,
    /** Argument list too long */
    ArgListTooLong = E2BIG,
    /** Bad file number */
    InvalidFileHandle = EBADF,
    /** No children */
    NoChildren = ECHILD,
    /** Not enough memory */
    NotEnoughMemory = ENOMEM,
    /** Permission denied */
    AccessDenied = EACCES,
    /** Bad address */
    InvalidAddress = EFAULT,
    /** Device or resource busy */
    Busy = EBUSY,
    /** File exists */
    FileExists = EEXIST,
    /** Cross-device link */
    InvalidLink = EXDEV,
    /** No such device */
    DeviceNotFound = ENODEV,
    /** Not a directory */
    NotADirectory = ENOTDIR,
    /** Is a directory */
    IsDirectory = EISDIR,
    /** Invalid argument */
    InvalidArgument = EINVAL,
    /** Too many open files in system */
    TooManyOpenFiles = ENFILE,
    /** File descriptor value too large */
    DescriptorTooLarge = EMFILE,
    /** File too large */
    FileTooLarge = EFBIG,
    /** No space left on device */
    OutOfDiskSpace = ENOSPC,
    /** Illegal seek */
    InvalidSeek = ESPIPE,
    /** Read-only file system */
    ReadOnlyFs = EROFS,
    /** Too many links */
    TooManyLinks = EMLINK,
    /** Result too large */
    OutOfRange = ERANGE,
    /** Deadlock */
    Deadlock = EDEADLK,
    /** No lock */
    NoLock = ENOLCK,
    /** A non blocking operation could not be immediately completed */
    WouldBlock = ENODATA,
    /** Operation timed out. */
    Timeout = ETIME,
    /** Protocol error */
    ProtocolError = EPROTO,
    /** Bad message */
    InvalidMessage = EBADMSG,
    /** Inappropriate file type or format */
    InvalidFileFormat = EFTYPE,
    /** Function not implemented */
    NotImplemented = ENOSYS,
    /** Directory not empty */
    DirectoryNotEmpty = ENOTEMPTY,
    /** File or path name too long */
    PathTooLong = ENAMETOOLONG,
    /** Too many symbolic links */
    LinkCycle = ELOOP,
    /** Operation not supported */
    NotSupported = EOPNOTSUPP,
    /** Protocol family not supported  */
    ProtocolNotSupported = EPFNOSUPPORT,
    /** No buffer space available */
    BufferNotAvailable = ENOBUFS,
    /** Protocol not available */
    ProtocolNotAvailable = ENOPROTOOPT,
    /** Unknown protocol */
    UnknownProtocol = EPROTONOSUPPORT,
    /** Illegal byte sequence */
    InvalidByteSequence = EILSEQ,
    /** Value too large for defined data type */
    Overflow = EOVERFLOW,
    /** Operation canceled */
    Cancelled = ECANCELED,

};

/**
 * @brief Macro for verification whether passed OSResult value indicates success.
 */
static inline bool OS_RESULT_SUCCEEDED(OSResult x)
{
    return x == OSResult::Success;
}

/**
 * @brief Macro for verification whether passed OSResult value indicates failure.
 */
static inline bool OS_RESULT_FAILED(OSResult x)
{
    return x != OSResult::Success;
}

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
 * Task priorites
 */
enum class TaskPriority
{
    Idle = 0, //!< Idle
    P1,       //!< P1
    P2,       //!< P2
    P3,       //!< P3
    P4,       //!< P4
    P5,       //!< P5
    P6,       //!< P6
    P7,       //!< P7
    P8,       //!< P8
    P9,       //!< P9
    P10,      //!< P10
    P11,      //!< P11
    P12,      //!< P12
    P13,      //!< P13
    P14,      //!< P14
    Highest   //!< Highest
};

/**
 * @brief Definition of operating system interface.
 */
class System : public PureStatic
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
        TaskPriority priority,
        OSTaskHandle* taskHandle);

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
     * @param[in] eventGroup Handle to the event group that should be updated.
     * @param[in] bitsToChange Bits that should be cleared.
     * @return The value of the event group before the specified bits were cleared.
     */
    static OSEventBits EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange);

    /**
     * @brief Suspends current task execution until the specific bits in the event group are set.
     *
     * @param[in] eventGroup The affected event group handle.
     * @param[in] bitsToWaitFor Bits that the caller is interested in.
     * @param[in] waitAll Flat indicating whether this procedure should return when all requested bits are set.
     * @param[in] autoReset Flag indicating whether the signaled bits should be cleared on function return.
     * @param[in] timeout Operation timeout in ms.
     *
     * @return The value of the event group at the time either the event bits being waited for became set,
     * or the block time expired.
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

/**
 * RTOS Task wrapper
 */
template <typename Param, std::size_t StackSize, TaskPriority Priority> class Task
{
    static_assert(sizeof(Param) < 16, "WTF are you trying to do?");
    static_assert(StackSize <= UINT16_MAX, "Stack size must be uint16_t number");

  public:
    /**
     * @brief Type of function that can be used as task handler
     */
    using HandlerType = void (*)(Param);

    /**
     * @brief Initializes (but not creates in RTOS) task
     * @param[in] name Task name
     * @param[in] param Parameter passed to task
     * @param[in] handler Function that will be executed in new task
     */
    Task(const char* name, Param param, HandlerType handler) : _taskName(name), _param(param), _handler(handler), _handle(nullptr)
    {
    }

    /**
     * Creates RTOS task
     * @return Operation status
     */
    OSResult Create()
    {
        return System::CreateTask(EntryPoint, this->_taskName, StackSize, static_cast<void*>(this), Priority, &this->_handle);
    }

  private:
    /**
     * @brief Wrapper function that dispatches newly started task to specified handler
     * @param param
     */
    static void EntryPoint(void* param)
    {
        auto This = static_cast<Task*>(param);
        This->_handler(This->_param);
    }

    /** @brief Task name */
    const char* _taskName;
    /** @brief Parameter passed to task */
    Param _param;
    /** @brief Task handler */
    HandlerType _handler;
    /** @brief Task handle */
    OSTaskHandle _handle;
};

/** @}*/

#endif
