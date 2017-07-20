#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SPI/SPIMock.h"
#include "base/writer.h"
#include "fm25w/fm25w.hpp"
#include "mock/fm25w.hpp"
#include "system.h"
#include "utils.hpp"

using testing::StrictMock;
using testing::Eq;
using testing::ElementsAre;
using testing::ContainerEq;
using testing::DoAll;
using testing::Return;
using std::uint16_t;
using std::uint8_t;
using testing::InSequence;

using namespace devices::fm25w;
namespace
{
    Status operator|(Status a, Status b)
    {
        return static_cast<Status>(num(a) | num(b));
    }

    class FM25WDriverTest : public testing::Test
    {
      protected:
        FM25WDriverTest();

        StrictMock<SPIInterfaceMock> _spi;
        FM25WDriver _driver;
    };

    FM25WDriverTest::FM25WDriverTest() : _driver(_spi)
    {
    }

    TEST_F(FM25WDriverTest, ShouldReadStatusRegister)
    {
        auto expected = Status::WriteEnabled | Status::WriteProtect1;

        InSequence s;
        {
            auto s = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(0b00000101)));
            EXPECT_CALL(this->_spi, Read(SpanOfSize(1))).WillOnce(DoAll(FillBuffer<0>(num(expected)), Return(OSResult::Success)));
        }

        auto status = _driver.ReadStatus();

        ASSERT_THAT(status, Eq(expected));
    }

    TEST_F(FM25WDriverTest, ShouldReadSpecifiedRange)
    {
        uint16_t address = 0x2BCD;
        std::array<uint8_t, 16> data{15, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2};

        InSequence s;
        {
            auto s = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(num(Command::Read), 0x2B, 0xCD)));
            EXPECT_CALL(this->_spi, Read(SpanOfSize(16))).WillOnce(DoAll(FillBuffer<0>(data), Return(OSResult::Success)));
        }

        decltype(data) result{0};

        _driver.Read(address, result);

        ASSERT_THAT(result, ContainerEq(data));
    }

    TEST_F(FM25WDriverTest, ShouldWriteSpecifiedRange)
    {
        uint16_t address = 0x2BCD;
        std::array<uint8_t, 16> data{15, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2};
        InSequence s;
        {
            auto s = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(num(Command::EnableWrite))));
        }

        {
            auto s = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(num(Command::Write), 0x2B, 0xCD)));
            EXPECT_CALL(this->_spi, Write(Eq(gsl::span<const uint8_t>(data))));
        }

        _driver.Write(address, data);
    }
}
