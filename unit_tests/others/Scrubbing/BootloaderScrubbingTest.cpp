#include <algorithm>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/flash_driver.hpp"
#include "msc/msc.hpp"
#include "program_flash/boot_table.hpp"
#include "scrubber/bootloader.hpp"
#include "shared.hpp"

using testing::_;
using testing::A;
using testing::Eq;
using testing::Return;
using testing::ElementsAre;
using testing::Invoke;

struct MCUFlashMock : public drivers::msc::IMCUFlash
{
    MOCK_METHOD1(Erase, void(std::size_t offset));
    MOCK_METHOD2(Program, void(std::size_t offset, gsl::span<std::uint8_t> buffer));
    MOCK_CONST_METHOD0(Begin, std::uint8_t const*());
};

class BootloaderScrubbingTest : public testing::Test
{
  public:
    BootloaderScrubbingTest();

  protected:
    testing::NiceMock<FlashDriverMock> _flash;
    program_flash::BootTable _bootTable;

    std::array<std::uint8_t, 64_KB> _mcuMemory;
    testing::NiceMock<MCUFlashMock> _mcuFlash;

    scrubber::BootloaderScrubber _scrubber;
};

BootloaderScrubbingTest::BootloaderScrubbingTest() : _bootTable(_flash), _scrubber(ScrubbingBuffer, this->_bootTable, _mcuFlash)
{
    this->_mcuMemory.fill(0x5A);

    ON_CALL(this->_mcuFlash, Begin()).WillByDefault(Return(this->_mcuMemory.data()));

    ON_CALL(this->_mcuFlash, Erase(_)).WillByDefault(Invoke([this](std::size_t offset) {
        auto sector = gsl::make_span(this->_mcuMemory.begin() + offset, 4_KB);
        std::fill(sector.begin(), sector.end(), 0xFF);
    }));

    ON_CALL(this->_mcuFlash, Program(_, _)).WillByDefault(Invoke([this](std::size_t offset, gsl::span<std::uint8_t> buffer) {
        std::copy(buffer.begin(), buffer.end(), this->_mcuMemory.begin() + offset);
    }));
}

TEST_F(BootloaderScrubbingTest, ShouldDoNothingIfAllCopiesAreAlright)
{
    EXPECT_CALL(this->_flash, EraseSector(_)).Times(0);
    EXPECT_CALL(this->_flash, Program(_, A<gsl::span<const uint8_t>>())).Times(0);
}

TEST_F(BootloaderScrubbingTest, ShouldRewriteInvalidCopies)
{
    auto set = [this](std::uint8_t copy, std::uint8_t value1, std::uint8_t value2, std::uint8_t value3) {
        this->_bootTable.GetBootloaderCopy(copy).Write(1_KB, {&value1, 1});
        this->_bootTable.GetBootloaderCopy(copy).Write(2_KB, {&value2, 1});
        this->_bootTable.GetBootloaderCopy(copy).Write(3_KB, {&value3, 1});
    };

    auto ptr = &this->_bootTable.GetBootloaderCopy(4).Content()[1_KB];

    set(0, 0xAA, 0x00, 0x11);
    set(1, 0x22, 0xBB, 0xCC);
    set(2, 0xAA, 0x33, 0xCC);
    set(3, 0x44, 0xBB, 0x55);
    set(4, 0xAA, 0xBB, 0xCC);

    ASSERT_THAT(*ptr, Eq(0xAA));
    this->_scrubber.Scrub();

    auto get = [this](std::uint8_t copy) -> std::array<const std::uint8_t, 3> {
        auto content = this->_bootTable.GetBootloaderCopy(copy).Content();
        return {content[1_KB], content[2_KB], content[3_KB]};
    };

    ASSERT_THAT(get(0), ElementsAre(0xAA, 0xBB, 0xCC));
    ASSERT_THAT(get(1), ElementsAre(0xAA, 0xBB, 0xCC));
    ASSERT_THAT(get(2), ElementsAre(0xAA, 0xBB, 0xCC));
    ASSERT_THAT(get(3), ElementsAre(0xAA, 0xBB, 0xCC));
    ASSERT_THAT(get(4), ElementsAre(0xAA, 0xBB, 0xCC));

    ASSERT_THAT(this->_scrubber.Status().CopiesCorrected, Eq(4U));
}

TEST_F(BootloaderScrubbingTest, ShouldRewriteInvalidMCUPages)
{
    auto set = [this](std::uint8_t copy, std::uint8_t value1, std::uint8_t value2, std::uint8_t value3) {
        this->_bootTable.GetBootloaderCopy(copy).Write(0_KB, {&value1, 1});
        this->_bootTable.GetBootloaderCopy(copy).Write(4_KB, {&value2, 1});
        this->_bootTable.GetBootloaderCopy(copy).Write(8_KB, {&value3, 1});
    };

    for (auto i = 0; i < 5; i++)
    {
        set(i, 0xAA, 0xBB, 0xCC);
    }

    this->_scrubber.Scrub();

    ASSERT_THAT(this->_mcuMemory[0_KB], Eq(0xAA));
    ASSERT_THAT(this->_mcuMemory[4_KB], Eq(0xBB));
    ASSERT_THAT(this->_mcuMemory[8_KB], Eq(0xCC));
}
