#ifndef LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_
#define LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_

#include "base/os.h"
#include "gsl/span"

namespace experiments
{
    /**
     * @defgroup experiments Experiments infrastructure
     *
     * @{
     */

    /**
     * @brief Type used to represent experiment code
     */
    using ExperimentCode = std::uint8_t;

    /**
     * @brief Experiment start result
     */
    enum class StartResult : std::uint8_t
    {
        Success, //!< Success
        Failure  //!< Failure
    };

    /**
     * @brief Experiment iteration result
     */
    enum class IterationResult : std::uint8_t
    {
        None,             //!< No iterations yet
        Finished,         //!< Experiment is finished
        LoopImmediately,  //!< Immediately run next iteration
        WaitForNextCycle, //!< Run next iteration when next mission loop cycle begins
        Failure           //!< Experiment failed - abort
    };

    /**
     * @brief Experiment state
     */
    struct ExperimentState
    {
        /** @brief Code of requested experiment */
        Option<ExperimentCode> RequestedExperiment;
        /** @brief Code of currently performed experiment */
        Option<ExperimentCode> CurrentExperiment;
        /** @brief Last experiment start result */
        Option<StartResult> LastStartResult;
        /** @brief Result of last experiment iteration */
        Option<IterationResult> LastIterationResult;
        /** @brief Experiment iteration counter */
        std::uint32_t IterationCounter;
    };

    /**
     * @brief Experiment interface
     */
    struct IExperiment
    {
        /**
         * @brief Returns experiment type
         * @return Experiment type
         */
        virtual ExperimentCode Type() = 0;
        /**
         * @brief Performs experiment start
         * @return Start result
         */
        virtual StartResult Start() = 0;

        /**
         * @brief Performs single experiment iteration
         * @return Disposition about next iteration
         */
        virtual IterationResult Iteration() = 0;

        /**
         * @brief Performs experiment clean-up
         * @param lastResult Result of last iteration
         */
        virtual void Stop(IterationResult lastResult) = 0;
    };

    /**
     * @brief Interface of object responsible for controlling currently performed experiment.
     */
    struct IExperimentController
    {
        /**
         * @brief Dumps current experiment state.
         * @return Current experiment execution state.
         */
        virtual ExperimentState CurrentState() = 0;
    };

    /**
     * @brief Coordinates experiment execution
     */
    class ExperimentController final : public IExperimentController
    {
      public:
        /**
         * @brief Default ctor
         */
        ExperimentController();

        /**
         * @brief Sets list of available experiments
         * @param experiments Experiments
         */
        void SetExperiments(gsl::span<IExperiment*> experiments);

        /**
         * @brief Performs initialization
         */
        void Initialize();

        /**
         * @brief Requests experiment to be started in next mission loop
         * @param experiment Experiment type
         * @retval true Experiment request was accepted
         * @retval false Experiment request was denied (other experiment is in progress)
         */
        bool RequestExperiment(ExperimentCode experiment);

        /**
         * @brief Aborts current experiment immediately
         */
        void AbortExperiment();

        /**
         * @brief Background task procedure
         */
        void BackgroundTask();

        /**
         * @brief Checks if experiment is in progress
         * @return true if experiment is in progress
         */
        bool InProgress();

        /**
         * @brief Checks if any experiment was requested
         * @return true if experiment was requested
         */
        bool IsExperimentRequested();

        /**
         * @brief Starts experiment
         */
        void StartExperiment();

        /**
         * @brief Notifies experiment loop about start of mission loop iteration
         */
        void NotifyLoopIterationStart();

        virtual ExperimentState CurrentState() final override;

        /**
         * @brief Events used in synchronization
         */
        struct Event
        {
            /** @brief InProgress flag */
            static constexpr OSEventBits InProgress = 1 << 0;
            /** @brief Abort requested flag */
            static constexpr OSEventBits AbortRequest = 1 << 1;
            /** @brief Mission loop iteration started */
            static constexpr OSEventBits MissionLoopIterationStarted = 1 << 2;
            /** @brief Request notification on next mission loop start flag */
            static constexpr OSEventBits MissionLoopNotificationReqested = 1 << 3;
        };

      private:
        /**
         * @brief Performs single experiment
         * @param experiment Experiment to run
         */
        void RunExperiment(IExperiment& experiment);

        /** @brief List of available experiments */
        gsl::span<IExperiment*> _experiments;

        /** @brief Lock */
        OSSemaphoreHandle _sync;

        /** @brief Event group used for status reporting */
        EventGroup _event;
        /** @brief Queue holding requested experiment */
        Queue<ExperimentCode, 1> _queue;

        /** @brief Requested experiment */
        Option<ExperimentCode> _requestedExperiment;

        /** @brief Current experiment */
        Option<ExperimentCode> _currentExperiment;

        /** @brief Last experiment start result */
        Option<StartResult> _lastStartResult;

        /** @brief Last experiment iteration result */
        Option<IterationResult> _lastIterationResult;

        /** @brief Experiment iterations counter */
        std::uint32_t _iterationCounter;

        /** @brief Background task */
        Task<ExperimentController*, 4_KB, TaskPriority::P3> _task;
    };

    /** @} */
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_ */
