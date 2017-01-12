#include <algorithm>
#include <cstdint>
#include <gsl/span>
#include "comm/Frame.hpp"
#include "comm/ITransmitFrame.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "telecommand_handling/telecommand_handling.h"
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
using devices::comm::ITransmitFrame;
using namespace telecommands::handling;

struct TeleCommandDepsMock : public IDecryptFrame, public IDecodeTelecommand
{
    MOCK_METHOD2(Decrypt, DecryptFrameResult(span<const uint8_t>, span<uint8_t>));

    MOCK_METHOD1(Decode, DecodeTelecommandResult(span<const uint8_t>));
};

struct TeleCommandHandlerMock : public IHandleTeleCommand
{
    MOCK_METHOD2(Handle, void(ITransmitFrame&, span<const uint8_t> parameters));
    MOCK_CONST_METHOD0(CommandCode, uint8_t());
};

struct TransmitFrameMock : public ITransmitFrame
{
    MOCK_METHOD1(SendFrame, bool(span<const uint8_t>));
};

class TeleCommandHandlingTest : public Test
{
  public:
    TeleCommandHandlingTest();

  protected:
    IncomingTelecommandHandler handling;
    NiceMock<TeleCommandDepsMock> deps;
    TransmitFrameMock transmitFrame;
};

TeleCommandHandlingTest::TeleCommandHandlingTest() : handling(deps, deps, span<IHandleTeleCommand*, 0>())
{
}

TEST_F(TeleCommandHandlingTest, IncomingFrameShouldBeDecryptedAndDecoded)
{
    std::uint8_t buffer[40] = {};
    Frame frame(0, 0, 0, buffer);
    EXPECT_CALL(this->deps, Decrypt(_, _)).WillOnce(Return(DecryptFrameResult::Success(frame.Payload())));
    EXPECT_CALL(this->deps, Decode(_)).WillOnce(Return(DecodeTelecommandResult::Success(0xA, frame.Payload().subspan(1))));

    this->handling.HandleFrame(this->transmitFrame, frame);
}

TEST_F(TeleCommandHandlingTest, HandlerShouldBeCalledForKnownTelecommand)
{
    std::uint8_t buffer[40] = "ABCD";
    Frame frame(0, 0, 0, buffer);

    EXPECT_CALL(this->deps, Decrypt(_, _)).WillOnce(Invoke([](span<const uint8_t> frame, span<uint8_t> decrypted) {
        auto lastCopied = std::copy(frame.cbegin(), frame.cend(), decrypted.begin());

        auto decryptedDataLength = lastCopied - decrypted.begin();

        return DecryptFrameResult::Success(decrypted.subspan(0, decryptedDataLength));
    }));
    EXPECT_CALL(this->deps, Decode(_)).WillOnce(Invoke([](span<const uint8_t> frame) {
        return DecodeTelecommandResult::Success(frame[0], frame.subspan(1, frame.length() - 1));
    }));

    NiceMock<TeleCommandHandlerMock> someCommand;
    EXPECT_CALL(someCommand, Handle(_, _));
    EXPECT_CALL(someCommand, CommandCode()).WillRepeatedly(Return(static_cast<uint8_t>('A')));

    IHandleTeleCommand* commands[] = {&someCommand};

    IncomingTelecommandHandler handler(deps, deps, span<IHandleTeleCommand*>(commands));

    handler.HandleFrame(this->transmitFrame, frame);
}

TEST_F(TeleCommandHandlingTest, WhenDecryptionFailsShouldNotAttemptFrameDecoding)
{
    EXPECT_CALL(this->deps, Decrypt(_, _)).WillOnce(Return(DecryptFrameResult::Failure(DecryptFrameFailureReason::GeneralError)));

    EXPECT_CALL(this->deps, Decode(_)).Times(0);

    Frame frame;

    this->handling.HandleFrame(this->transmitFrame, frame);
}

TEST_F(TeleCommandHandlingTest, WhenDecodingFrameShouldNotAttemptInvokingHandler)
{
    const uint8_t frameContents[] = {0xA, 0xB};

    EXPECT_CALL(this->deps, Decrypt(_, _)).WillOnce(Return(DecryptFrameResult::Success(frameContents)));

    EXPECT_CALL(this->deps, Decode(_)).WillOnce(Return(DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason::GeneralError)));

    NiceMock<TeleCommandHandlerMock> someCommand;
    EXPECT_CALL(someCommand, Handle(_, _)).Times(0);

    IHandleTeleCommand* telecommands[] = {&someCommand};

    IncomingTelecommandHandler handler(this->deps, this->deps, span<IHandleTeleCommand*>(telecommands));

    Frame frame;

    handler.HandleFrame(this->transmitFrame, frame);
}
