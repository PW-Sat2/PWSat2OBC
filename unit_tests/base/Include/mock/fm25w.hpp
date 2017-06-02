#include "gmock/gmock.h"
#include "fm25w/fm25w.hpp"

struct FM25WDriverMock : public devices::fm25w::IFM25WDriver
{
    FM25WDriverMock();
    ~FM25WDriverMock();

    MOCK_METHOD0(ReadStatus, Option<devices::fm25w::Status>());

    MOCK_METHOD2(Read, void(devices::fm25w::Address address, gsl::span<std::uint8_t> buffer));

    MOCK_METHOD2(Write, void(devices::fm25w::Address address, gsl::span<const std::uint8_t> buffer));
};
