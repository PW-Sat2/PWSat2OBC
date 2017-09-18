#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_NOTIFYTIMECHANGEDMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_NOTIFYTIMECHANGEDMOCK_HPP_

#pragma once

#include "gmock/gmock.h"
#include "mission/base.hpp"

struct NotifyTimeChangedMock : mission::INotifyTimeChanged
{
    NotifyTimeChangedMock();

    ~NotifyTimeChangedMock();

    MOCK_METHOD1(NotifyTimeChanged, void(std::chrono::milliseconds));
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_NOTIFYTIMECHANGEDMOCK_HPP_ */
