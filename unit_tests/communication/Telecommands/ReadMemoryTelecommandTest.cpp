#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/writer.h"
#include "mock/comm.hpp"
#include "obc/telecommands/memory.hpp"
#include "utils.hpp"

namespace
{
    using namespace obc::telecommands;
    using telecommunication::downlink::DownlinkAPID;
    using telecommunication::downlink::CorrelatedDownlinkFrame;
    using testing::_;
    using testing::ElementsAre;

    class ReadMemoryTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> _transmitter;
        ReadMemoryTelecommand _telecommand;
    };

    TEST_F(ReadMemoryTelecommandTest, ShouldReadMemoryRange)
    {
        std::array<std::uint8_t, 300> memoryToRead;
        for (auto i = 0U; i < memoryToRead.size(); i++)
        {
            memoryToRead[i] = 11 + (i % 256);
        }

        auto part1 = gsl::make_span(memoryToRead).subspan(0, CorrelatedDownlinkFrame::MaxPayloadSize);
        auto part2 = gsl::make_span(memoryToRead).subspan(CorrelatedDownlinkFrame::MaxPayloadSize);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::MemoryContent, 0, 0x12, part1)));
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::MemoryContent, 1, 0x12, part2)));

        std::array<std::uint8_t, 9> args;
        Writer w(args);
        w.WriteByte(0x12);
        w.WriteDoubleWordLE(reinterpret_cast<std::uint32_t>(memoryToRead.data()));
        w.WriteDoubleWordLE(300);

        _telecommand.Handle(_transmitter, args);
    }

    TEST_F(ReadMemoryTelecommandTest, ShouldReadMemoryRangeFrom0)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::MemoryContent, 0, 0x12, SpanOfSize(100))));

        std::array<std::uint8_t, 9> args;
        Writer w(args);
        w.WriteByte(0x12);
        w.WriteDoubleWordLE(0);
        w.WriteDoubleWordLE(100);

        _telecommand.Handle(_transmitter, args);
    }

    TEST_F(ReadMemoryTelecommandTest, ShouldRespondWithErrorOnTooShortFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, 0x12, ElementsAre(1))));

        std::array<std::uint8_t, 1> args;
        Writer w(args);
        w.WriteByte(0x12);

        _telecommand.Handle(_transmitter, args);
    }

    TEST_F(ReadMemoryTelecommandTest, ShouldTrimReadedSizeToUpperMemoryLimit)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::MemoryContent, 0, 0x12, SpanOfSize(50))));

        std::array<std::uint8_t, 9> args;
        Writer w(args);
        w.WriteByte(0x12);
        w.WriteDoubleWordLE(std::numeric_limits<std::size_t>::max() - 50);
        w.WriteDoubleWordLE(100);

        _telecommand.Handle(_transmitter, args);
    }
}
