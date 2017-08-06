#ifndef LIBS_MISSION_BASE_HPP
#define LIBS_MISSION_BASE_HPP

#pragma once

#include <array>
#include <cstdint>
#include "system.h"

namespace mission
{
    /**
     * @addtogroup mission_loop
     * @{
     */

    /**
     * @brief Tag type used for marking the type as action that can be performed during the mission.
     *
     * Inherit from it to mark the type as action. Type that inherits from it should define following non static member:
     * @code{.cpp}
     * ActionDescriptor<T> BuildAction();
     * @endcode
     * The purpose of this function is to provide the mission loop runner unified interface to run all of the actions.
     * The T type parameter is used to specify the mission state type on which the action implementation operates.
     *
     * Mission action is the portion of the mission implementation that determines whether the specific action should be
     * performed and either entirely perform it or at least initiate it. The actions should provide two entry points to
     * their logic:
     * - Condition Procedure - Pointer to the procedure that determines whether the action should be performed at current
     * mission stage. This procedure based on the current mission state should be able to determine whether the action
     * is needed.
     * - Action Procedure - Pointer to the procedure that either performs the current action or initializes it as someone
     * else is responsible for performing it. This procedure should not perform any long running operations or wait for
     * long time periods as it can interfere with global system operation.
     */
    struct Action
    {
    };

    /**
     * @brief Tag type used for marking the type as mission state update action that can be performed during the mission.
     *
     * Inherit from it to mark the type as state updater. Type that inherits from it should define following non static member:
     *
     * @code{.cpp}
     * UpdateDescriptor<T> BuildUpdate();
     * @endcode
     * The purpose of this function is to provide the mission loop runner unified interface to run all of the update actions.
     * The T type parameter is used to specify the mission state type on which the action implementation operates.
     *
     * The state update action is supposed to observe its designed part of the software/hardware and based on those observations
     * update global mission state object. Although it is not forbidden update actions should not initiate any operations
     * that may in any way change the logical state of the software/hardware part this this action operates on.
     */
    struct Update
    {
    };

    /**
     * @brief Tag type used for marking the type as mission state validator that is executed for checking current mission state.
     *
     * Inherit from it to mark the type as state validator. Type that inherits from it should define following non static member:
     * @code{.cpp}
     * VerifyDescriptor<T> BuildVerify();
     * @endcode
     * The purpose of this function is to provide the mission loop runner unified interface to run all of the update actions.
     * The T type parameter is used to specify the mission state type on which the action implementation operates.
     *
     * The purpose of the state verification action is to analyze global mission state in terms of contradictions.
     * One validation action should concentrate on the logically independent portion of the state and look for the
     * contradicting state in itself or contradiction in context of the corresponding part of the software/hardware.
     */
    struct Verify
    {
    };

    /**
     * @brief Tag type used for marking the type as task that is disabled from autostart
     * state.
     *
     * Inherit from it to mark the type as disabled from autostart. Type that inherits from it should define following non static member:
     * @code{.cpp}
     * void AutostartEnable();
     * @endcode
     * The purpose of this function is to provide the mission loop runner unified interface to enable all disabled tasks.
     *
     * The purpose of the disabling is that automatic starting of several tasks interferes with tests.
     * So Mission loop starts all of tasks, but those marked as that type should be in the disabled state until mission loop enables them
     * at higher runlevel.
     *
     * The disabling is done at task's level, there is no generic mechanism to force that.
     */
    struct AutostartDisabled
    {
    };

    /**
     * @brief Enumerator of mission state update statuses.
     */
    enum class UpdateResult
    {
        /** @brief State updated successfully */
        Ok,
        /** @brief Non-critical error during update */
        Warning,
        /** @brief Fatal error during update */
        Failure
    };

    /**
     * @brief Type of procedure that updates mission state.
     * @param[in,out] state System state object to update.
     * @param[in] param Arbitrary pointer that points to action specific context.
     * @tparam State Type of the state that the procedure operates on.
     * @return State update result.
     */
    template <typename State> using UpdateProc = UpdateResult (*)(State& state, void* param);

    /**
     * @brief Structure that describes mission update action entry point.
     *
     * @tparam State Type of the state on which the described action operates on.
     */
    template <typename State> struct UpdateDescriptor
    {
        /**
         * @brief Action Name.
         *
         * Used only for debugging purposes.
         */
        const char* name;

        /**
         * @brief Pointer to action entry point.
         */
        UpdateProc<State> updateProc;

        /**
         * @brief Arbitrary parameter passed to update procedure.
         */
        void* param;

        /**
         * @brief performs system state update
         * @param state System state
         * @return Update result
         */
        UpdateResult Execute(State& state) const;
    };

    template <typename State> inline UpdateResult UpdateDescriptor<State>::Execute(State& state) const
    {
        if (this->updateProc == nullptr)
        {
            return UpdateResult::Warning;
        }

        return this->updateProc(state, this->param);
    }

    /**
     * @brief Enumerator of all possible state verification results.
     */
    enum class VerifyResult
    {
        /** @brief No error. State is correct */
        Ok,
        /** @brief State is invalid. */
        Failure
    };

    /**
     * @brief Result of state verification
     */
    class VerifyDescriptorResult final
    {
      public:
        /** default ctor. */
        constexpr VerifyDescriptorResult();

        /**
         * @brief ctor.
         * @param[in] result State verification result.
         * @param[in] reason Detailed verification status.
         */
        constexpr VerifyDescriptorResult(VerifyResult result, std::uint32_t reason);

        /**
         * @brief Returns verification result.
         * @return Verification result.
         */
        constexpr VerifyResult Result() const noexcept;

        /**
         * @brief Returns Detailed verification status.
         * @return Detailes status.
         */
        constexpr std::uint32_t Reason() const noexcept;

      private:
        /** @brief Result of state verification */
        VerifyResult result;

        /** @brief Detailed reason of verification result */
        std::uint32_t reason;
    };

    constexpr VerifyDescriptorResult::VerifyDescriptorResult() //
        : VerifyDescriptorResult(VerifyResult::Ok, 0)
    {
    }

    constexpr VerifyDescriptorResult::VerifyDescriptorResult(VerifyResult result, std::uint32_t reason) //
        : result(result), reason(reason)
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
     * @param[in] state State object that should be validated.
     * @param[in] param Pointer to action specific context.
     * @tparam State Type of the state that current action operates on.
     * @returns Verification result.
     */
    template <typename State> using VerifyProc = VerifyDescriptorResult (*)(const State& state, void* param);

    /**
     * @brief Structure that describes the action that is responsible for verifying state object.
     * @tparam State Type of the state object this action operates on.
     */
    template <typename State> struct VerifyDescriptor
    {
        /**
         * @brief Action Name.
         *
         * Used only for debugging purposes.
         */
        const char* name;

        /**
         * @brief Pointer to state verification action entry point.
         */
        VerifyProc<State> verifyProc;

        /**
         *  @brief Parameter passed to state verification action.
         */
        void* param;
    };

    /**
     * @brief Type of the procedure that is entry point to executive part of state update action.
     * @param[in] state Reference to the state object.
     * @param[in] param Pointer to action specific context.
     * @tparam State Type of the state this action operates on.
     */
    template <typename State> using ActionProc = void (*)(State& state, void* param);

    /**
     * @brief Type of the procedure that verifies whether or not action should be performed in current state.
     * @param[in] state Reference to the state object.
     * @param[in] param Pointer to action specific context.
     * @return Decision if procedure can be performed.
     * @retval true The action should be performed on current mission stage.
     * @retval false The action should not be performed on current mission stage.
     */
    template <typename State> using ConditionProc = bool (*)(const State& state, void* param);

    /**
     * @brief Structure that describes mission action.
     * @tparam State Type of the state this action operates on.
     */
    template <typename State> struct ActionDescriptor
    {
        /**
         * @brief Name of action
         *
         * Used only for debugging purposes.
         */
        const char* name;

        /**
         *  @brief Pointer to procedure that is an entry point to he executive part of this action.
         */
        ActionProc<State> actionProc;

        /**
         * @brief Pointer to procedure that decides if action can be performed
         */
        ConditionProc<State> condition;

        /**
         * @brief Pointer to action specific context object that is  passed to both action & condition procedures.
         */
        void* param;

        /**
         * @brief Evaluates condition for this action
         * @param state System state
         * @return true if action is runnable
         */
        inline bool EvaluateCondition(const State& state);

        /**
         * @brief Executes action
         * @param state System state
         */
        inline void Execute(State& state);
    };

    template <typename State> inline bool ActionDescriptor<State>::EvaluateCondition(const State& state)
    {
        if (this->condition == nullptr)
        {
            return true;
        }

        return this->condition(state, this->param);
    }

    template <typename State> inline void ActionDescriptor<State>::Execute(State& state)
    {
        if (this->actionProc != nullptr)
        {
            this->actionProc(state, this->param);
        }
    }

    /**
     * @brief Packs number of actions into single action that can plug into mission loop
     * @tparam State System state
     * @tparam Number of actions
     *
     * @remark Checking if inner action can be executed is done in "Invoke actions" phase of mission loop
     */
    template <typename State, std::size_t Count> class CompositeAction : public Action
    {
      public:
        /**
         * @brief Constructs composite action from given action descriptors
         * @param name Name for composite action descriptor
         * @param descriptors Action descriptors
         */
        template <typename... T> CompositeAction(const char* name, T... descriptors) : _name(name), _descriptors{descriptors...}
        {
            static_assert(sizeof...(T) == Count, "Descriptors count must equal template parameter");
        }

        /**
         * @brief Builds action
         * @return Action descriptor for composite action
         */
        ActionDescriptor<State> BuildAction();

      private:
        /**
         * @brief Dummy condition - always returns true
         * @param state Not used
         * @param param Not used
         * @return Always true
         */
        static bool CanRun(const State& state, void* param);
        /**
         * @brief Executes all runnable actions from composite action
         * @param state System state
         * @param param Pointer to CompositeAction object
         */
        static void Run(State& state, void* param);

        /** @brief Composite action name */
        const char* _name;

        /**
         * @brief Inner actions descriptors
         */
        std::array<ActionDescriptor<State>, Count> _descriptors;
    };

    template <typename State, std::size_t Count> bool CompositeAction<State, Count>::CanRun(const State& state, void* param)
    {
        UNUSED(state, param);
        return true;
    }

    template <typename State, std::size_t Count> void CompositeAction<State, Count>::Run(State& state, void* param)
    {
        auto This = reinterpret_cast<CompositeAction<State, Count>*>(param);

        for (auto d : This->_descriptors)
        {
            if (d.EvaluateCondition(state))
            {
                d.Execute(state);
            }
        }
    }

    template <typename State, std::size_t Count> ActionDescriptor<State> CompositeAction<State, Count>::BuildAction()
    {
        ActionDescriptor<State> n;
        n.param = this;
        n.name = this->_name;
        n.actionProc = Run;
        n.condition = CanRun;

        return n;
    }
}

#endif
