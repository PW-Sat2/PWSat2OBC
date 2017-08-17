#ifndef MOCK_SETUP_SAIL_EXPERIMENT_HPP
#define MOCK_SETUP_SAIL_EXPERIMENT_HPP

#include "gmock/gmock.h"
#include "experiment/sail/sail.hpp"

class SetupSailExperimentMock : public experiment::sail::ISetupSailExperiment
{
  public:
    SetupSailExperimentMock() = default;
    ~SetupSailExperimentMock() = default;

    MOCK_METHOD1(SetExperimentEnd, void(std::chrono::milliseconds));
    MOCK_METHOD1(SetNextTelemetryAcquisition, void(std::chrono::milliseconds));
    MOCK_METHOD1(SetNextPhotoTaken, void(std::chrono::milliseconds));
    MOCK_METHOD1(SetCorrelationId, void(uint8_t));
};

#endif /* MOCK_SETUP_SAIL_EXPERIMENT_HPP */
