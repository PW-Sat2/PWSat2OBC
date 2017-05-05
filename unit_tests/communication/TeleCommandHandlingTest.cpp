#include <algorithm>
#include <cstdint>
#include <gsl/span>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "comm/Frame.hpp"
#include "comm/ITransmitter.hpp"
#include "mock/comm.hpp"
#include "telecommunication/telecommand_handling.h"
#include "utils.hpp"

using std::uint8_t;
using std::size_t;
using gsl::span;

using testing::Test;
using testing::NiceMock;
using testing::Return;
using testing::Invoke;
using testing::_;
using testing::Eq;
using testing::StrEq;

using devices::comm::Frame;
using devices::comm::ITransmitter;
using namespace telecommunication::uplink;
namespace
{
    struct TeleCommandDepsMock : public IDecodeTelecommand
    {
        MOCK_METHOD1(Decode, DecodeTelecommandResult(span<const uint8_t>));
    };

    struct TeleCommandHandlerMock : public IHandleTeleCommand
    {
        MOCK_METHOD2(Handle, void(ITransmitter&, span<const uint8_t> parameters));
        MOCK_CONST_METHOD0(CommandCode, uint8_t());
    };

    class TeleCommandHandlingTest : public Test
    {
      public:
        TeleCommandHandlingTest();

      protected:
        IncomingTelecommandHandler handling;
        NiceMock<TeleCommandDepsMock> deps;
        TransmitterMock transmitter;
    };

    TeleCommandHandlingTest::TeleCommandHandlingTest() : handling(deps, span<IHandleTeleCommand*, 0>())
    {
    }

    TEST_F(TeleCommandHandlingTest, IncomingFrameShouldBeDecoded)
    {
        std::uint8_t buffer[40] = {};
        Frame frame(0, 0, 0, buffer);
        EXPECT_CALL(this->deps, Decode(_)).WillOnce(Return(DecodeTelecommandResult::Success(0xA, frame.Payload().subspan(1))));

        this->handling.HandleFrame(this->transmitter, frame);
    }

    TEST_F(TeleCommandHandlingTest, HandlerShouldBeCalledForKnownTelecommand)
    {
        std::uint8_t buffer[40] = "ABCD";
        Frame frame(0, 0, 0, buffer);

        EXPECT_CALL(this->deps, Decode(_)).WillOnce(Invoke([](span<const uint8_t> frame) {
            return DecodeTelecommandResult::Success(frame[0], frame.subspan(1, frame.length() - 1));
        }));

        NiceMock<TeleCommandHandlerMock> someCommand;
        EXPECT_CALL(someCommand, Handle(_, _));
        EXPECT_CALL(someCommand, CommandCode()).WillRepeatedly(Return(static_cast<uint8_t>('A')));

        IHandleTeleCommand* commands[] = {&someCommand};

        IncomingTelecommandHandler handler(deps, span<IHandleTeleCommand*>(commands));

        handler.HandleFrame(this->transmitter, frame);
    }

    TEST_F(TeleCommandHandlingTest, WhenDecodingFrameShouldNotAttemptInvokingHandler)
    {
        EXPECT_CALL(this->deps, Decode(_)).WillOnce(Return(DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::GeneralError)));

        NiceMock<TeleCommandHandlerMock> someCommand;
        EXPECT_CALL(someCommand, Handle(_, _)).Times(0);

        IHandleTeleCommand* telecommands[] = {&someCommand};

        IncomingTelecommandHandler handler(this->deps, span<IHandleTeleCommand*>(telecommands));

        Frame frame;

        handler.HandleFrame(this->transmitter, frame);
    }
}
