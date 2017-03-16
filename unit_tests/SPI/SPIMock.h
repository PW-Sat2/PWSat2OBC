#ifndef UNIT_TESTS_SPI_SPIMOCK_H_
#define UNIT_TESTS_SPI_SPIMOCK_H_

#include <gsl/span>
#include <gmock/gmock.h>

#include "spi/spi.h"

struct SPIInterfaceMock;

class SPIExpectSelected
{
  public:
    SPIExpectSelected(SPIInterfaceMock& mock);
    ~SPIExpectSelected();

  private:
    SPIInterfaceMock& _mock;
};

struct SPIInterfaceMock : drivers::spi::ISPIInterface
{
    SPIInterfaceMock();

    MOCK_METHOD0(Select, void());
    MOCK_METHOD0(Deselect, void());
    MOCK_METHOD1(Write, void(gsl::span<const std::uint8_t> buffer));
    MOCK_METHOD1(Read, void(gsl::span<std::uint8_t> buffer));

    SPIExpectSelected ExpectSelected();
};

#endif /* UNIT_TESTS_SPI_SPIMOCK_H_ */
