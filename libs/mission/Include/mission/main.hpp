#ifndef LIBS_MISSION_INCLUDE_MISSION_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_MISSION_H_

#pragma once

#include <array>
#include <type_traits>
#include "base.hpp"
#include "base/os.h"
#include "gsl/span"
#include "logic.hpp"
#include "traits.hpp"

/**
 * @defgroup MissionLoop MissionLoop state
 *
 */
namespace mission
{
    /**
     * @defgroup mission MissionLoop module
     *
     * This module contains entire mission management related log.
     * @{
     */

    struct GetUpdateDescriptor
    {
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildUpdate();
        }
    };

    struct GetActionDescriptor
    {
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildAction();
        }
    };

    struct GetVerifyDescriptor
    {
        template <typename T, typename U> static void Apply(T& object, U& target)
        {
            target = object.BuildVerify();
        }
    };

    template <typename State, typename... T> struct MissionLoop final : public T...
    {
      public:
        template <typename Type> using IsAction = std::is_base_of<Action, Type>;
        template <typename Type> using IsUpdate = std::is_base_of<Update, Type>;
        template <typename Type> using IsVerify = std::is_base_of<Verify, Type>;

        static constexpr std::uint32_t CountAction = CountHelper<IsAction, T...>::value;

        static constexpr std::uint32_t CountUpdate = CountHelper<IsUpdate, T...>::value;

        static constexpr std::uint32_t CountVerify = CountHelper<IsVerify, T...>::value;

        typedef std::array<ActionDescriptor<State>, CountAction> ActionList;

        typedef std::array<UpdateDescriptor<State>, CountUpdate> UpdateList;

        typedef std::array<VerifyDescriptor<State>, CountVerify> VerifyList;

        template <typename... Args> MissionLoop(Args&&... args);

        /**
         * @brief Initializes mission-control task.
         */
        bool Initialize();

        void Suspend();

        void Resume();

        void RunOnce();

      private:
        static constexpr std::uint32_t PauseRequestFlag = 0x1;

        static constexpr std::uint32_t PauseAckFlag = 0x2;

        template <size_t i, template <typename Type> class Pred, typename Action, typename Collection, typename P, typename... U>
        void Process(Collection& collection, std::true_type);

        template <size_t i, template <typename Type> class Pred, typename Action, typename Collection> void Process(...);

        void TaskLoop();

        static void MissionLoopControlTask(void* param);

        State state;

        ActionList actions;

        UpdateList updates;

        VerifyList verifications;

        TaskHandle_t taskHandle;

        OSEventGroupHandle eventGroup;
    };

    template <typename State, typename... T>
    template <typename... Args>
    MissionLoop<State, T...>::MissionLoop(Args&&... args) //
        : T(std::forward<Args>(args))...,
          taskHandle(nullptr),
          eventGroup(nullptr)
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
                System::CreateTask(MissionLoopControlTask, "MissionLoopControl", 2048, this, TaskPriority::P2, &this->taskHandle)))
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
            System::EventGroupWaitForBits(this->eventGroup, PauseAckFlag, false, true, MAX_DELAY);
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
        ActionList runnableActions;
        std::array<VerifyDescriptorResult, CountVerify> detailedVerifyResult;
        LOG(LOG_LEVEL_TRACE, "Updating system state");

        auto updateResult = SystemStateUpdate(state, gsl::span<UpdateDescriptor<State>>(updates));

        LOGF(LOG_LEVEL_TRACE, "System state update result %d", static_cast<int>(updateResult));

        auto verifyResult = SystemStateVerify(state,                 //
            gsl::span<const VerifyDescriptor<State>>(verifications), //
            gsl::span<VerifyDescriptorResult>(detailedVerifyResult));

        LOGF(LOG_LEVEL_TRACE, "Verify result %d", static_cast<int>(verifyResult));

        auto runableSpan = SystemDetermineActions(state, //
            gsl::span<ActionDescriptor<State>>(actions), //
            gsl::span<ActionDescriptor<State>>(runnableActions));

        LOGF(LOG_LEVEL_TRACE, "Executing %d actions", static_cast<int>(runableSpan.size()));

        SystemDispatchActions(state, runableSpan);
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
            const OSEventBits result = System::EventGroupWaitForBits(this->eventGroup, PauseRequestFlag, false, true, 10000);
            if (result == PauseRequestFlag)
            {
                LOG(LOG_LEVEL_WARNING, "MissionLoop task paused");
                System::EventGroupSetBits(this->eventGroup, PauseAckFlag);
                System::SuspendTask(NULL);
            }
            else
            {
                RunOnce();
            }
        }
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_MISSION_H_ */
