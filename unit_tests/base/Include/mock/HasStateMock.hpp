#ifndef UNIT_TESTS_MOCK_HAS_STATE_HPP_
#define UNIT_TESTS_MOCK_HAS_STATE_HPP_

#include "gmock/gmock.h"
#include "base/IHasState.hpp"

template <typename State> struct HasStateMock : public IHasState<State>
{
    typedef typename std::remove_reference<State>::type StateType;

    HasStateMock() = default;

    ~HasStateMock() = default;

    MOCK_METHOD0_T(MockGetState, StateType&());

    MOCK_CONST_METHOD0_T(MockGetState, const StateType&());

    virtual const StateType& GetState() const noexcept override
    {
        return MockGetState();
    }

    virtual StateType& GetState() noexcept override
    {
        return MockGetState();
    }
};

#endif /* UNIT_TESTS_MOCK_HAS_STATE_HPP_ */
