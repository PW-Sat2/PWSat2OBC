#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "leuart/line_io.h"
#include "utils.h"

/**
 * @defgroup terminal Terminal module
 *
 * @{
 */

/**
 * @brief Type of function used as terminal command handler
 */
using TerminalCommandHandler = void (*)(std::uint16_t argc, char* argv[]);

/**
 * Structure describing single terminal command
 */
struct TerminalCommandDescription
{
    /** @brief Name of the command */
    const char* name;
    /** Pointer to handler function */
    TerminalCommandHandler handler;
};

/**
 * @brief Terminal class
 */
class Terminal
{
  public:
    /**
     * @brief Initializes new @ref Terminal object
     * @param[in] stdio Reference to object responsible for text-based I/O
     */
    Terminal(LineIO& stdio);

    /**
     * @brief Sets supported commands list
     * @param[in] commands Commands list
     */
    void SetCommandList(gsl::span<const TerminalCommandDescription> commands);

    /**
     * @brief Starts terminal
     */
    void Initialize();

    /**
     * @brief Prints new lin
     */
    void NewLine();

    /**
     * @brief Prints formatted text
     * @param[in] text Text to print with format placeholders
     */
    void Printf(const char* text, ...);

    /**
     * @brief Prints text
     * @param[in] text Text to print
     */
    void Puts(const char* text);

    /**
     * @brief Print not-terminated buffer
     * @param[in] buffer Buffer to print
     */
    void PrintBuffer(gsl::span<const char> buffer);

  private:
    /**
     * @brief Sends prefix
     */
    void SendPrefix();

    /**
     * @brief Handles incoming command
     * @param[in] buffer Buffer with incoming command
     *
     * After this method ends, @p buffer is no longer usable
     */
    void HandleCommand(char* buffer);

    /**
     * @brief Terminal loop
     * @param[in] Terminal object
     */
    static void Loop(Terminal*);

    /**
     * @brief Reference to object responsible for text-based I/O
     */
    LineIO& _stdio;

    /**
     * @brief RTOS task with main loop
     */
    Task<Terminal*, 2_KB, TaskPriority::P4> _task;

    /**
     * @brief Command list
     */
    gsl::span<const TerminalCommandDescription> _commandList;
};

/** @} */

#endif
