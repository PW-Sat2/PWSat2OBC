#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "experiment/program/program_exp.hpp"
#include "mock/comm.hpp"
#include "mock/flash_driver.hpp"
#include "program_flash/boot_table.hpp"

using testing::Eq;
using testing::Ne;

namespace
{
    class CopyBootSlotsExperimentTest : public testing::Test
    {
      protected:
        testing::NiceMock<FlashDriverMock> _flash;
        testing::NiceMock<TransmitterMock> _transmitter;
        program_flash::BootTable _bootTable{_flash};

        experiment::program::CopyBootSlotsExperiment _exp{_bootTable, _flash, _transmitter};
    };

    TEST_F(CopyBootSlotsExperimentTest, ShouldCopyBootSlots)
    {
        this->_bootTable.Initialize();

        auto entry0 = _bootTable.Entry(0);
        auto entry1 = _bootTable.Entry(1);
        auto entry2 = _bootTable.Entry(2);

        auto entry3 = _bootTable.Entry(3);
        auto entry4 = _bootTable.Entry(4);

        entry0.Erase();
        entry1.Erase();
        entry2.Erase();

        entry0.Description("DEF");
        entry1.Description("ABC");
        entry2.Description("ABC");

        entry3.Description("To be erased");
        entry4.Description("To be erased2");

        std::bitset<6> source(0b000111);
        std::bitset<6> target(0b011000);

        _exp.SetupEntries(source, target);

        _exp.Start();
        auto r = _exp.Iteration();

        ASSERT_THAT(r, Eq(experiments::IterationResult::Finished));

        ASSERT_THAT(entry4.WholeEntry(), Eq(entry2.WholeEntry()));
        ASSERT_THAT(entry3.WholeEntry(), Eq(entry2.WholeEntry()));
    }
}
