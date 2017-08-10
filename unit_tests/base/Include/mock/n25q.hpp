#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_N25Q_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_N25Q_HPP_

#include <gmock/gmock.h>
#include "n25q/n25q.h"

struct N25QDriverMock : public devices::n25q::IN25QDriver
{
    N25QDriverMock();
    ~N25QDriverMock();

    MOCK_METHOD2(ReadMemory, OSResult(std::size_t address, gsl::span<uint8_t> buffer));

    MOCK_METHOD3(BeginWritePage, devices::n25q::OperationWaiter(size_t address, ptrdiff_t offset, gsl::span<const uint8_t> page));

    MOCK_METHOD1(BeginEraseSubSector, devices::n25q::OperationWaiter(size_t address));

    MOCK_METHOD1(BeginEraseSector, devices::n25q::OperationWaiter(size_t address));

    MOCK_METHOD0(BeginEraseChip, devices::n25q::OperationWaiter());

    MOCK_METHOD0(Reset, devices::n25q::OperationResult());

    MOCK_METHOD2(WaitForOperation, devices::n25q::OperationResult(std::chrono::milliseconds timeout, devices::n25q::FlagStatus status));
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_N25Q_HPP_ */
