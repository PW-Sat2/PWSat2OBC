#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/flash_driver.hpp"
#include "program_flash/boot_table.hpp"
#include "scrubber/program.hpp"
#include "shared.hpp"

using testing::_;
using testing::A;
using testing::Eq;
using testing::Return;

class ProgramScrubbingTest : public testing::Test
{
  public:
    ProgramScrubbingTest();

  protected:
    testing::NiceMock<FlashDriverMock> _flash;
    program_flash::BootTable _bootTable;

    scrubber::ProgramScrubber _scrubber;
};

ProgramScrubbingTest::ProgramScrubbingTest() : _bootTable(this->_flash), _scrubber(ScrubbingBuffer, this->_bootTable, this->_flash, 0b111)
{
}

TEST_F(ProgramScrubbingTest, ShouldDoNothingIfAllSlotsAreCorrect)
{
    EXPECT_CALL(this->_flash, EraseSector(_)).Times(0);
    EXPECT_CALL(this->_flash, Program(_, A<gsl::span<const uint8_t>>())).Times(0);
}

TEST_F(ProgramScrubbingTest, ShouldRewriteSlotsThatAreInvalid)
{
    this->_bootTable.Entry(0).WriteContent(1_KB, std::array<uint8_t, 1>{0xAA});
    this->_bootTable.Entry(0).WriteContent(2_KB, std::array<uint8_t, 1>{0x00});
    this->_bootTable.Entry(0).WriteContent(3_KB, std::array<uint8_t, 1>{0xCC});

    this->_bootTable.Entry(1).WriteContent(1_KB, std::array<uint8_t, 1>{0x11});
    this->_bootTable.Entry(1).WriteContent(2_KB, std::array<uint8_t, 1>{0xBB});
    this->_bootTable.Entry(1).WriteContent(3_KB, std::array<uint8_t, 1>{0xCC});

    this->_bootTable.Entry(2).WriteContent(1_KB, std::array<uint8_t, 1>{0xAA});
    this->_bootTable.Entry(2).WriteContent(2_KB, std::array<uint8_t, 1>{0xBB});
    this->_bootTable.Entry(2).WriteContent(3_KB, std::array<uint8_t, 1>{0xCC});

    this->_scrubber.ScrubSlots();

    auto get = [this](std::uint8_t slot, std::size_t offset) { return *(this->_bootTable.Entry(slot).Content() + offset); };

    ASSERT_THAT(get(0, 1_KB), Eq(0xAA));
    ASSERT_THAT(get(0, 2_KB), Eq(0xBB));
    ASSERT_THAT(get(0, 3_KB), Eq(0xCC));

    ASSERT_THAT(get(1, 1_KB), Eq(0xAA));
    ASSERT_THAT(get(1, 2_KB), Eq(0xBB));
    ASSERT_THAT(get(1, 3_KB), Eq(0xCC));

    ASSERT_THAT(get(2, 1_KB), Eq(0xAA));
    ASSERT_THAT(get(2, 2_KB), Eq(0xBB));
    ASSERT_THAT(get(2, 3_KB), Eq(0xCC));

    ASSERT_THAT(this->_scrubber.Status().SlotsCorrected, Eq(2U));
}

TEST_F(ProgramScrubbingTest, ShouldAbortScrubbingIfUnableToTakeLock)
{
    EXPECT_CALL(this->_flash, Lock(_)).WillOnce(Return(false));
    EXPECT_CALL(this->_flash, Program(_, A<gsl::span<const uint8_t>>())).Times(0);

    this->_scrubber.ScrubSlots();
}
