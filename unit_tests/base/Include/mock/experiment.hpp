#ifndef UNIT_TESTS_BASE_MOCK_EXPERIMENT_HPP_
#define UNIT_TESTS_BASE_MOCK_EXPERIMENT_HPP_

#include "gmock/gmock.h"
#include "experiments/experiments.h"

struct ExperimentControllerMock : experiments::IExperimentController
{
    ExperimentControllerMock();
    ~ExperimentControllerMock();

    MOCK_METHOD0(CurrentState, experiments::ExperimentState());
    MOCK_METHOD1(RequestExperiment, bool(experiments::ExperimentCode experiment));
    MOCK_METHOD0(AbortExperiment, void());
};

#endif /* UNIT_TESTS_BASE_MOCK_EXPERIMENT_HPP_ */
