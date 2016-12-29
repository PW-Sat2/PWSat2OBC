#include <gmock/gmock.h>

#include "SPIMock.h"

SPIExpectSelected::SPIExpectSelected(SPIInterfaceMock& mock) : _mock(mock)
{
    EXPECT_CALL(_mock, Select()).Times(1);
}

SPIExpectSelected::~SPIExpectSelected()
{
    EXPECT_CALL(_mock, Deselect()).Times(1);
}

SPIExpectSelected SPIInterfaceMock::ExpectSelected()
{
    return SPIExpectSelected(*this);
}
