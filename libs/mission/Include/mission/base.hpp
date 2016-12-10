#ifndef LIBS_MISSION_BASE_HPP
#define LIBS_MISSION_BASE_HPP

#pragma once

#include <cstdint>

/**
 * @defgroup Mission Mission state
 *
 */

namespace mission
{
    struct Action
    {
    };

    struct Update
    {
    };

    struct Verify
    {
    };

    /**
     * @brief Result of updating state
     */
    enum class UpdateResult
    {
        /** @brief State updated successfully */
        UpdateOK,
        /** @brief Non-critical error during update */
        UpdateWarning,
        /** @brief Fatal error during update */
        UpdateFailure
    };

    /**
     * @brief Type of procedure that updates state
     * @param[in,out] state System state to update
     * @param[in] param Arbitrary parameter
     * @return State update result
     */
    template <typename State> using UpdateProc = UpdateResult (*)(State& state, void* param);

    /**
     * @brief Update descriptor
     */
    template <typename State> struct UpdateDescriptor
    {
        /** @brief Name */
        const char* name;
        /** @brief Pointer to procedure that updates state */
        UpdateProc<State> updateProc;
        /** @brief Arbitrary parameter passed to update procedure */
        void* param;
    };

    /**
         * @brief Result of state verification
         */
    enum class VerifyResult
    {
        /** @brief No error. State is correct */
        VerifyOK,
        /** @brief State is invalid */
        VerifyFailure
    };

    /**
     * @brief Result of state verification
     */
    class VerifyDescriptorResult
    {
      public:
        /** default ctor. */
        constexpr VerifyDescriptorResult();

        /**
         * @brief ctor
         * @param[in] result Result of state verification.
         * @param[in] reason Detailed verification status.
         */
        constexpr VerifyDescriptorResult(VerifyResult result, std::uint32_t reason);

        constexpr VerifyResult Result() const noexcept;

        constexpr std::uint32_t Reason() const noexcept;

      private:
        /** @brief Result of state verification */
        VerifyResult result;

        /** @brief Detailed reason of verification result */
        std::uint32_t reason;
    };

    constexpr VerifyDescriptorResult::VerifyDescriptorResult() //
        : VerifyDescriptorResult(VerifyResult::VerifyOK, 0)
    {
    }

    constexpr VerifyDescriptorResult::VerifyDescriptorResult(VerifyResult result, std::uint32_t reason) //
        : result(result),
          reason(reason)
    {
    }

    constexpr inline VerifyResult VerifyDescriptorResult::Result() const noexcept
    {
        return this->result;
    }

    constexpr inline std::uint32_t VerifyDescriptorResult::Reason() const noexcept
    {
        return this->reason;
    }

    /**
     * @brief Procedure that performs state verification
     * @param[in] state State to verify
     * @param[in] param Arbitrary parameter
     * @returns Verification result
     */
    template <typename State> using VerifyProc = VerifyDescriptorResult (*)(const State& state, void* param);

    /**
     * @brief State verification descriptor
     */
    template <typename State> struct VerifyDescriptor
    {
        /** @brief Name */
        const char* name;
        /** @brief Pointer to procedure that performs verification */
        VerifyProc<State> verifyProc;
        /** @brief Parameter passed to verify procedure */
        void* param;
    };

    /**
     * @brief Procedure that performs some action
     * @param[in] state System state
     * @param[in] param Arbitrary parameter
     */
    template <typename State> using ActionProc = void (*)(const State& state, void* param);

    /**
     * @brief Procedure that decides if action can be performed
     * @param[in] state System state
     * @param[in] param Arbitrary parameter
     * @return Decision if procedure can be performed
     */
    template <typename State> using ConditionProc = bool (*)(const State& state, void* param);

    /**
     * @brief Action descriptor
     */
    template <typename State> struct ActionDescriptor
    {
        /** @brief Name of action */
        const char* name;
        /** @brief Pointer to procedure that performs action */
        ActionProc<State> actionProc;
        /** @brief Pointer to procedure that decides if action can be performed */
        ConditionProc<State> condition;
        /** @brief Parameter passed to ActionProc and Condition */
        void* param;
    };
}

#endif
