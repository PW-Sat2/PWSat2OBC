#ifndef SRC_TERMINAL_H_
#define SRC_TERMINAL_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "line_io.h"
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
    Terminal(ILineIO& stdio);

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
    void Printf(const char* text, ...) __attribute__((format(printf, 2, 3)));

    /**
     * @brief Prints text
     * @param[in] text Text to print
     */
    void Puts(const char* text);

    /**
     * @brief Print not-terminated buffer
     * @param[in] buffer Buffer to print
     */
    void PrintBuffer(gsl::span<const std::uint8_t> buffer);

    /**
     * @brief Reads number of bytes
     * @param outputBuffer Buffer that will be sent before reading inputBuffer
     * @param inputBuffer Buffer that will be filled
     */
    void ExchangeBuffers(gsl::span<const std::uint8_t> outputBuffer, gsl::span<std::uint8_t> inputBuffer);

  private:
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
    ILineIO& _stdio;

    /**
     * @brief RTOS task with main loop
     */
    Task<Terminal*, 6_KB, TaskPriority::Highest> _task;

    /**
     * @brief Command list
     */
    gsl::span<const TerminalCommandDescription> _commandList;
};

/**
 * @brief Support class for reading lengthy data from terminal in parts
 *
 * Transfer is performed using following protocol:
 * Host (H), Device (D)
 * @verbatim
 * D: #
 * H: <Data length (32-bit, LE)>
 * D: <Part length (32-bit LE)>
 * H: <Requested number of bytes>
 * D: <Part length (32-bit LE)>
 * H: <Requested number of bytes>
 * ...
 * @endverbatim
 */
class TerminalPartialRetrival
{
  public:
    /**
     * @brief Initializes @see TerminalPartialRetrival instance
     * @param terminal Terminal
     * @param buffer Buffer used to store parts
     */
    TerminalPartialRetrival(Terminal& terminal, gsl::span<uint8_t> buffer);

    /**
     * @brief Initializes transfer
     */
    void Start();

    /**
     * @brief Reads single part from terminal
     * @retval None of no more data to be retrieved
     * @retval Some with span containing data read in current part
     */
    Option<gsl::span<uint8_t>> ReadPart();

  private:
    /** @brief Terminal */
    Terminal& _terminal;
    /** @brief Buffer used to store parts */
    gsl::span<uint8_t> _buffer;
    /** @brief Remaining data length */
    std::size_t _remainingLength;
};

/** @} */

#endif
