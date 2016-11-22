#include <algorithm>
#include <cstdint>
#include <gsl/span>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "comm/comm.h"
#include "telecommand_handling/telecommand_handling.h"
#include "utils.hpp"

using testing::Test;
using testing::NiceMock;
using testing::Return;
using testing::Invoke;
using testing::_;
using testing::Eq;
using testing::StrEq;
using namespace std;
using namespace devices::comm;
using namespace telecommands::handling;

struct TeleCommandDepsMock : public IDecryptFrame, public IDecodeTelecommand
{
    MOCK_METHOD3(Decrypt, DecryptStatus(gsl::span<const uint8_t>, gsl::span<uint8_t>, size_t&));

    MOCK_METHOD3(Decode, DecodeFrameStatus(gsl::span<const uint8_t>, uint8_t&, gsl::span<const uint8_t>&));
};

struct TeleCommandHandlerMock : public IHandleTeleCommand
{
    MOCK_METHOD1(Handle, void(gsl::span<const std::uint8_t> parameters));
    MOCK_CONST_METHOD0(CommandCode, uint8_t());
};

class TeleCommandHandlingTest : public Test
{
  public:
    TeleCommandHandlingTest();

  protected:
    IncomingTelecommandHandler handling;
    NiceMock<TeleCommandDepsMock> deps;
};

TeleCommandHandlingTest::TeleCommandHandlingTest() : handling(deps, deps, gsl::span<IHandleTeleCommand*, 0>())
{
}

TEST_F(TeleCommandHandlingTest, IncomingFrameShouldBeDecryptedAndDecoded)
{
    CommFrame frame;
    frame.Size = 40;

    EXPECT_CALL(this->deps, Decrypt(_, _, _)).WillOnce(Return(DecryptStatus::Success));
    EXPECT_CALL(this->deps, Decode(_, _, _)).WillOnce(Return(DecodeFrameStatus::Success));

    this->handling.HandleFrame(frame);
}

TEST_F(TeleCommandHandlingTest, HandlerShouldBeCalledForKnownTelecommand)
{
    const char* contents = "ABCD";

    CommFrame frame;
    frame.Size = 50;
    strcpy((char*)frame.Contents, contents);

    EXPECT_CALL(this->deps, Decrypt(_, _, _))
        .WillOnce(Invoke([](gsl::span<const uint8_t> frame, gsl::span<uint8_t> decrypted, size_t& decryptedDataLength) {
            auto lastCopied = std::copy(frame.cbegin(), frame.cend(), decrypted.begin());

            decryptedDataLength = lastCopied - decrypted.begin();

            return DecryptStatus::Success;
        }));
    EXPECT_CALL(this->deps, Decode(_, _, _))
        .WillOnce(Invoke([](gsl::span<const std::uint8_t> frame, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& parameters) {
            commandCode = frame[0];
            parameters = frame.subspan(1, frame.length() - 1);

            return DecodeFrameStatus::Success;
        }));

    NiceMock<TeleCommandHandlerMock> someCommand;
    EXPECT_CALL(someCommand, Handle(_));
    EXPECT_CALL(someCommand, CommandCode()).WillRepeatedly(Return(static_cast<uint8_t>('A')));

    IHandleTeleCommand* commands[] = {&someCommand};

    IncomingTelecommandHandler handler(deps, deps, gsl::span<IHandleTeleCommand*>(commands));

    handler.HandleFrame(frame);
}

TEST_F(TeleCommandHandlingTest, WhenDecryptionFailsShouldNotAttemptFrameDecoding)
{
    EXPECT_CALL(this->deps, Decrypt(_, _, _)).WillOnce(Return(DecryptStatus::Failed));

    EXPECT_CALL(this->deps, Decode(_, _, _)).Times(0);

    CommFrame frame;

    this->handling.HandleFrame(frame);
}

TEST_F(TeleCommandHandlingTest, WhenDecodingFrameShouldNotAttemptInvokingHandler)
{
    EXPECT_CALL(this->deps, Decrypt(_, _, _)).WillOnce(Return(DecryptStatus::Success));

    EXPECT_CALL(this->deps, Decode(_, _, _))
        .WillOnce(
            Invoke([](gsl::span<const std::uint8_t> /*frame*/, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& /*parameters*/) {
                commandCode = 0xAA;

                return (DecodeFrameStatus)DecodeFrameStatus::Failed;
            }));

    NiceMock<TeleCommandHandlerMock> someCommand;
    EXPECT_CALL(someCommand, Handle(_)).Times(0);

    IHandleTeleCommand* telecommands[] = {&someCommand};

    IncomingTelecommandHandler handler(this->deps, this->deps, gsl::span<IHandleTeleCommand*>(telecommands));

    CommFrame frame;

    handler.HandleFrame(frame);
}
