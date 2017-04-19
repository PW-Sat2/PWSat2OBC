#ifndef LIBS_MISSION_INCLUDE_MISSION_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_MISSION_H_

#pragma once

#include <array>
#include <type_traits>
#include "base.hpp"
#include "base/os.h"
#include "gsl/span"
#include "logger/logger.h"
#include "logic.hpp"
#include "traits.hpp"

using namespace std::chrono_literals;

namespace mission
{
    /**
     * @defgroup mission Mission
     *
     * @brief Satellite mission & state management and in addition all mission related actions.
     */

    /**
     * @defgroup mission_loop Mission Main Loop
     * @ingroup mission
     * @brief Concept implemented by this library is very similar to game loop.
     *
     * This mission loop implementation is based on three separate phases:
     *
     *  - \b Update - all update descriptors are executed. After that state contain the most accurate information
     * about overall satellite state. Examples: Time, power level from EPS, antenna status (opened or not)
     *  - \b Verify - Checks if state makes any sense. Examples of such invalid state are: negative time, antenna opened before
     * first 30 minutes passed, etc. It is possible that such state is result of malfunction of some device and needs further
     * investigation
     *  - \b Dispatch - List of runnable actions is determined based on their condition. After that they are executed one by one
     * @{
     */

    /**
     * @brief Type that extracts Update descriptor out of the passed object and assigns the result to passed placeholder.
     */
    struct GetUpdateDescriptor
    {
        /**
         * @brief Extracts the descriptor and assigns it to passed target.
         * @param[in] object Inspected object.
         * @param[out] target Placeholder for operation result.
         */
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildUpdate();
        }
    };

    /**
     * @brief Type that extracts Action descriptor out of the passed object and assigns the result to passed placeholder.
     */
    struct GetActionDescriptor
    {
        /**
         * @brief Extracts the descriptor and assigns it to passed target.
         * @param[in] object Inspected object.
         * @param[out] target Placeholder for operation result.
         */
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildAction();
        }
    };

    /**
     * @brief Type that extracts Verify descriptor out of the passed object and assigns the result to passed placeholder.
     */
    struct GetVerifyDescriptor
    {
        /**
         * @brief Extracts the descriptor and assigns it to passed target.
         * @param[in] object Inspected object.
         * @param[out] target Placeholder for operation result.
         */
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildVerify();
        }
    };

    /**
     * @brief Mission main loop implementation.
     *
     * This class is responsible for running mission main loop. The list of actions to run is specified via the
     * template argument parameter pack. Classification of the action to the Action/Update/Verify category is based on the
     * inheriting from the mission::Action, mission::Update, mission::Verify tag types.
     * @tparam State Mission state type.
     * @tparam T Parameter pack that defines currently supported actions. All actions from this list should be able to
     * operate on a state whose type is State.
     */
    template <typename State, typename... T> struct MissionLoop final : public T...
    {
      public:
        /**
         * @brief Type of the current mission state object.
         */
        typedef typename std::remove_reference<State>::type StateType;

        /**
         * @brief Predicate that checks whether passed type is an Action.
         */
        template <typename Type> using IsAction = std::is_base_of<Action, Type>;

        /**
         * @brief Predicate that checks whether passed type is an Update action.
         */
        template <typename Type> using IsUpdate = std::is_base_of<Update, Type>;

        /**
         * @brief Predicate that checks whether passed type is a Verify action.
         */
        template <typename Type> using IsVerify = std::is_base_of<Verify, Type>;

        /**
         * @brief Constant that contains current number of Actions.
         */
        static constexpr std::uint32_t CountAction = CountHelper<IsAction, T...>::value;

        /**
         * @brief Constant that contains current number of Update actions.
         */
        static constexpr std::uint32_t CountUpdate = CountHelper<IsUpdate, T...>::value;

        /**
         * @brief Constant that contains current number of Verify actions.
         */
        static constexpr std::uint32_t CountVerify = CountHelper<IsVerify, T...>::value;

        /**
         * @brief Type of collection that contains action descriptors.
         */
        typedef std::array<ActionDescriptor<State>, CountAction> ActionList;

        /**
         * @brief Type of collection that contains update descriptors.
         */
        typedef std::array<UpdateDescriptor<State>, CountUpdate> UpdateList;

        /**
         * @brief Type of collection that contains verify descriptors.
         */
        typedef std::array<VerifyDescriptor<State>, CountVerify> VerifyList;

        /**
         * @brief ctor.
         *
         * This constructor assumes that all actions can be default constructed.
         */
        MissionLoop();

        /**
         * @brief ctor.
         * @param[in] args List of arguments for action initialization.
         *
         * This constructor initializes all actions using the passed arguments. For this to work all actions have
         * to be constructible by single argument constructor. First argument passed to this constructor will be passed
         * to the first action from the parameter pack list, second argument to this constructor will be passed to the
         * second action from the pack, and so on.
         */
        template <typename... Args> MissionLoop(Args&&... args);

        /**
         * @brief Initializes mission loop task.
         * @return Operation status, true on success, false otherwise.
         */
        bool Initialize();

        /**
         * @brief Suspends indefinitely execution of mission loop.
         */
        void Suspend();

        /**
         * @brief Resumes mission loop execution.
         */
        void Resume();

        /**
         * @brief Runs single mission loop iteration.
         */
        void RunOnce();

        /**
         * @brief Requests running single iteration of mission loop
         *
         * @remark Mission loop is executed in dedicated mission task
         */
        void RequestSingleIteration();

        /**
         * @brief Current mission state accessor.
         * @return Reference to current mission state.
         */
        const StateType& GetState() const noexcept;

        /**
         * @brief Current mission state accessor.
         * @return Reference to current mission state.
         */
        StateType& GetState() noexcept;

      private:
        /**
         * @brief Flag signaled when mission loop execution should be suspended.
         */
        static constexpr std::uint32_t PauseRequestFlag = 0x1;

        /**
         * @brief Flag signaled when mission loop execution should be resumed.
         */
        static constexpr std::uint32_t PauseAckFlag = 0x2;

        static constexpr std::uint32_t RunOnceRequestFlag = 0x4;

        static constexpr std::uint32_t RunOnceFinishedFlag = 0x8;

        /**
         * @brief Initializes all descriptor lists.
         */
        void Setup();

        template <size_t i, template <typename Type> class Pred, typename Action, typename Collection, typename P, typename... U>
        void Process(Collection& collection, std::true_type);

        template <size_t i, template <typename Type> class Pred, typename Action, typename Collection> void Process(...);

        /**
         * @brief Main mission loop.
         */
        void TaskLoop();

        /**
         * @brief Mission loop entry point.
         * @param[in] param Task context.
         */
        static void MissionLoopControlTask(void* param);

        /** Current mission state. */
        State state;

        /** List of currently executed actions. */
        ActionList actions;

        /** List of currently executed update actions. */
        UpdateList updates;

        /** List of currently used verification actions. */
        VerifyList verifications;

        /** Handle to system taks that executes the mission loop. */
        OSTaskHandle taskHandle;

        /** Handle to event group used for controlling the state of mission loop execution. */
        OSEventGroupHandle eventGroup;
    };

    template <typename State, typename... T> MissionLoop<State, T...>::MissionLoop() : taskHandle(nullptr), eventGroup(nullptr)
    {
        Setup();
    }

    template <typename State, typename... T>
    template <typename... Args>
    MissionLoop<State, T...>::MissionLoop(Args&&... args) //
        : T(std::forward<Args>(args))...,
          taskHandle(nullptr),
          eventGroup(nullptr)
    {
        static_assert(sizeof...(Args) == sizeof...(T), "Number of arguments must be equal to number of mission components");
        Setup();
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::Setup()
    {
        Process<0, IsUpdate, GetUpdateDescriptor, UpdateList, T...>(updates, HasMore<T...>());
        Process<0, IsAction, GetActionDescriptor, ActionList, T...>(actions, HasMore<T...>());
        Process<0, IsVerify, GetVerifyDescriptor, VerifyList, T...>(verifications, HasMore<T...>());
    }

    template <typename State, typename... T>
    template <size_t i, template <typename Type> class Pred, typename Action, typename Collection, typename Head, typename... Tail>
    inline void MissionLoop<State, T...>::Process(Collection& collection, std::true_type)
    {
        ApplyIf<Action, Pred<Head>::value>::Apply(static_cast<Head&>(*this), collection[i]);
        Process<i + AsInt<Pred<Head>::value>::value, Pred, Action, Collection, Tail...>(collection, HasMore<Tail...>());
    }

    template <typename State, typename... T>
    template <size_t i, template <typename Type> class Pred, typename Action, typename Collection>
    inline void MissionLoop<State, T...>::Process(...)
    {
    }

    template <typename State, typename... T> bool MissionLoop<State, T...>::Initialize()
    {
        this->eventGroup = System::CreateEventGroup();
        if (this->eventGroup == nullptr)
        {
            LOG(LOG_LEVEL_ERROR, "Unable to initialize mission state. Reason: event group.");
            return false;
        }

        if (OS_RESULT_FAILED(
                System::CreateTask(MissionLoopControlTask, "MissionLoopControl", 4_KB, this, TaskPriority::P2, &this->taskHandle)))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to initialize mission state. Reason: unable to create task. ");
            return false;
        }

        return true;
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::Suspend()
    {
        if (this->taskHandle != nullptr)
        {
            System::EventGroupSetBits(this->eventGroup, PauseRequestFlag);
            System::EventGroupWaitForBits(this->eventGroup, PauseAckFlag, false, true, InfiniteTimeout);
        }
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::Resume()
    {
        if (this->taskHandle != nullptr)
        {
            System::ResumeTask(this->taskHandle);
        }
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::RunOnce()
    {
        std::array<ActionDescriptor<State>*, CountAction> runnableActions;
        std::array<VerifyDescriptorResult, CountVerify> detailedVerifyResult;
        LOG(LOG_LEVEL_TRACE, "Updating system state");

        auto updateResult = SystemStateUpdate(state, gsl::make_span(updates));

        LOGF(LOG_LEVEL_TRACE, "System state update result %d", static_cast<int>(updateResult));

        auto verifyResult = SystemStateVerify(state,                 //
            gsl::span<const VerifyDescriptor<State>>(verifications), //
            gsl::make_span(detailedVerifyResult));

        LOGF(LOG_LEVEL_TRACE, "Verify result %d", static_cast<int>(verifyResult));

        auto runableSpan = SystemDetermineActions(state, //
            gsl::make_span(actions),                     //
            gsl::make_span(runnableActions));

        LOGF(LOG_LEVEL_TRACE, "Executing %d actions", static_cast<int>(runableSpan.size()));

        SystemDispatchActions(state, runableSpan);
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::RequestSingleIteration()
    {
        System::EventGroupSetBits(this->eventGroup, RunOnceRequestFlag | PauseRequestFlag);
        Resume();

        System::EventGroupWaitForBits(this->eventGroup, RunOnceFinishedFlag | PauseAckFlag, true, true, InfiniteTimeout);
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::MissionLoopControlTask(void* param)
    {
        auto missionState = static_cast<MissionLoop<State, T...>*>(param);
        missionState->TaskLoop();
    }

    template <typename State, typename... T> void MissionLoop<State, T...>::TaskLoop()
    {
        LOG(LOG_LEVEL_DEBUG, "Starting mission control task");
        for (;;)
        {
            const OSEventBits result =
                System::EventGroupWaitForBits(this->eventGroup, RunOnceRequestFlag | PauseRequestFlag, false, false, 10s);
            if (has_flag(result, RunOnceRequestFlag))
            {
                LOG(LOG_LEVEL_DEBUG, "Running mission loop task once");

                RunOnce();
                System::EventGroupClearBits(this->eventGroup, RunOnceRequestFlag);
                System::EventGroupSetBits(this->eventGroup, RunOnceFinishedFlag);
            }
            else if (has_flag(result, PauseRequestFlag))
            {
                LOG(LOG_LEVEL_WARNING, "MissionLoop task paused");
                System::EventGroupClearBits(this->eventGroup, PauseRequestFlag);
                System::EventGroupSetBits(this->eventGroup, PauseAckFlag);
                System::SuspendTask(nullptr);
            }
            else
            {
                RunOnce();
            }
        }
    }

    template <typename State, typename... T>
    inline const typename MissionLoop<State, T...>::StateType& MissionLoop<State, T...>::GetState() const noexcept
    {
        return this->state;
    }

    template <typename State, typename... T>
    inline typename MissionLoop<State, T...>::StateType& MissionLoop<State, T...>::GetState() noexcept
    {
        return this->state;
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_MISSION_H_ */
