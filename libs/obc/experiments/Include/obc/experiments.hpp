#ifndef LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_
#define LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_

#include <array>
#include <tuple>
#include "experiment/adcs/adcs.hpp"
#include "experiment/camera/camera_exp.hpp"
#include "experiment/fibo/fibo.h"
#include "experiment/flash/flash.hpp"
#include "experiment/leop/leop.hpp"
#include "experiment/payload/PayloadExperimentTelemetryProvider.hpp"
#include "experiment/payload/payload_exp.hpp"
#include "experiment/radfet/radfet.hpp"
#include "experiment/sail/sail.hpp"
#include "experiment/suns/suns.hpp"
#include "experiments/experiments.h"
#include "fs/fs.h"
#include "gpio/forward.h"
#include "payload/interfaces.h"
#include "photo/fwd.hpp"
#include "power/fwd.hpp"
#include "traits.hpp"

namespace obc
{
    /**
     * @defgroup obc_experiments Experiments
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief Helper class for holding set of experiments
     */
    template <typename... Experiments> class ExperimentsHolder
    {
      public:
        /**
         * @brief Ctor
         * @param experiments Experiments
         */
        constexpr ExperimentsHolder(Experiments... experiments)
            : _experiments{std::move(experiments)...}, _ptrs{&std::get<Experiments>(_experiments)...}
        {
        }

        /**
         * @brief Returns span of all experiments
         * @return Span of pointers for every experiment
         */
        inline gsl::span<experiments::IExperiment*> All()
        {
            return this->_ptrs;
        }

        /**
         * @brief Returns reference to single experiment
         * @return Reference to single experiment
         */
        template <typename Experiment> Experiment& Get()
        {
            return std::get<Experiment>(this->_experiments);
        }

      private:
        std::tuple<Experiments...> _experiments;
        std::array<experiments::IExperiment*, sizeof...(Experiments)> _ptrs;

        /**
         * @brief Checks if experiment codes are unique
         * @return true if experiment codes are unique
         */
        template <bool Tag, std::uint8_t Head, std::uint8_t... Rest> static constexpr bool AreCodesUnique()
        {
            if (IsValueInList<std::uint8_t>::IsInList<Head, Rest...>())
            {
                return false;
            }

            return AreCodesUnique<true, Rest...>();
        }

        /**
         * @brief Checks if experiment codes are unique (stop condition)
         * @return Always true
         */
        template <bool Tag> static constexpr bool AreCodesUnique()
        {
            return true;
        }

        static_assert(AreCodesUnique<true, Experiments::Code...>(), "Experiment codes must be unique");
    };

    /** @brief All OBC experiments */
    using AllExperiments = ExperimentsHolder<                 //
        experiment::fibo::FibonacciExperiment,                //
        experiment::adcs::DetumblingExperiment,               //
        experiment::leop::LaunchAndEarlyOrbitPhaseExperiment, //
        experiment::suns::SunSExperiment,                     //
        experiment::erase_flash::EraseFlashExperiment,        //
        experiment::radfet::RadFETExperiment,                 //
        experiment::sail::SailExperiment,                     //
        experiment::payload::PayloadCommissioningExperiment,  //
        experiment::camera::CameraCommissioningExperiment>;

    /**
     * @brief OBC experiments
     */
    class OBCExperiments
    {
      public:
        /**
         * @brief Ctor
         * @param fs File system
         * @param adcs ADCS coordinator
         * @param time Time provider
         * @param gyro Gyroscope Provider
         * @param powerControl Power control interface
         * @param suns Experimental SunS driver
         * @param payload Payload driver
         * @param n25q Redundant N25Q driver
         * @param transmitter Frame transmitter
         * @param[in] photoService Reference to object capable of taking photos
         * @param[in] sailState Pin connected to sail indicator
         * @param imtq IMTQ data provider
         * @param epsProvider Telemetry provider for Payload Experiment
         * @param errorCounterProvider Error Counter telmetry provider
         * @param temperatureProvider MCU telemetry provider
         */
        OBCExperiments(services::fs::IFileSystem& fs,
            adcs::IAdcsCoordinator& adcs,
            services::time::TimeProvider& time,
            devices::gyro::IGyroscopeDriver& gyro,
            services::power::IPowerControl& powerControl,
            devices::suns::ISunSDriver& suns,
            devices::payload::IPayloadDeviceDriver& payload,
            devices::n25q::RedundantN25QDriver& n25q,
            devices::comm::ITransmitter& transmitter,
            services::photo::IPhotoService& photoService,
            const drivers::gpio::Pin& sailState,
            telemetry::IImtqDataProvider& imtq,
            devices::eps::IEpsTelemetryProvider& epsProvider,
            error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
            temp::ITemperatureReader* temperatureProvider);

        /**
         * @brief Performs initialization
         */
        void InitializeRunlevel1();

        /**
         * @brief Checks if experiment codes are unique
         * @return true if experiment codes are unique
         */
        template <typename Experiment> Experiment& Get()
        {
            return this->Experiments.Get<Experiment>();
        }

        /** @brief Container with all experiments */
        AllExperiments Experiments;

        /** @brief Experiments controller */
        experiments::ExperimentController ExperimentsController;
    };

    /** @} */
}

#endif /* LIBS_OBC_EXPERIMENTS_INCLUDE_OBC_EXPERIMENTS_HPP_ */
