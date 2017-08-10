#include <array>

#include <gsl/span>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "utils.hpp"

#include "camera/camera_low_level.h"

using std::array;
using gsl::span;

using testing::Test;
using testing::Eq;
using testing::StrictMock;
using testing::ElementsAreArray;
using testing::Return;
using testing::DoAll;

using namespace devices::camera;
using namespace std::chrono_literals;

struct LineIOMock : public ILineIO
{
    MOCK_METHOD1(Puts, void(const char* s));

    MOCK_METHOD2(VPrintf, void(const char* text, va_list args));

    MOCK_METHOD1(PrintBuffer, void(gsl::span<const std::uint8_t> buffer));

    MOCK_METHOD3(Readline, size_t(char* buffer, size_t bufferLength, char promptChar));

    MOCK_METHOD2(Read, bool(gsl::span<std::uint8_t> buffer, std::chrono::milliseconds timeout));

    MOCK_METHOD3(ExchangeBuffers,
        bool(gsl::span<const std::uint8_t> outputBuffer, gsl::span<std::uint8_t> inputBuffer, std::chrono::milliseconds timeout));
};

static constexpr std::chrono::milliseconds DefaultTimeout = 5s;
static constexpr std::chrono::milliseconds ResetTimeout = std::chrono::milliseconds(100);

namespace commands
{
    using Command = std::array<uint8_t, 6>;

    template <CameraCmd CommandID> static constexpr Command Ack{0xAA, 0x0E, static_cast<uint8_t>(CommandID), 0x00, 0x00, 0x00};

    template <CameraCmd CommandID, uint8_t PackageID>
    static constexpr Command AckPackage{0xAA, 0x0E, static_cast<uint8_t>(CommandID), 0x00, 0x00, PackageID};

    static constexpr Command Sync{0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    static constexpr Command Reset{0xAA, 0x08, 0x00, 0x00, 0x00, 0xFF};
    static constexpr Command Invalid{0xAA, 0xFF, 0x00, 0x00, 0x00, 0x00};

    template <CameraJPEGResolution resolution>
    static constexpr Command Init{0xAA, 0x01, 0x00, 0x07, 0x07, static_cast<uint8_t>(resolution)};

    template <CameraPictureType::Enum type> static constexpr Command GetPicture{0xAA, 0x04, static_cast<uint8_t>(type), 0x00, 0x00, 0x00};

    template <CameraPictureType::Enum type, uint8_t lengthLow, uint8_t lengthMid, uint8_t lengthHigh>
    static constexpr Command Data{0xAA, 0x0A, static_cast<uint8_t>(type), lengthLow, lengthMid, lengthHigh};

    template <CameraSnapshotType type> static constexpr Command Snapshot{0xAA, 0x05, static_cast<uint8_t>(type), 0x00, 0x00, 0x00};

    template <uint8_t packageSizeLow, uint8_t packageSizeHigh>
    static constexpr Command SetPackageSize{0xAA, 0x06, 0x08, packageSizeLow, packageSizeHigh, 0x00};

    template <uint8_t firstDivider, uint8_t secondDivider>
    static constexpr Command SetBaudRate{0xAA, 0x07, firstDivider, secondDivider, 0x00, 0x00};
}

class LowLevelCameraTest : public Test
{
  public:
    LowLevelCameraTest() : _driver(_lineIOMock)
    {
    }

    void ExpectRequestAndResponse(                          //
        span<const uint8_t> command,                        //
        span<const uint8_t> response,                       //
        bool expectedResult = true,                         //
        std::chrono::milliseconds timeout = DefaultTimeout) //
    {
        EXPECT_CALL(_lineIOMock,
            ExchangeBuffers(                                                   //
                ElementsAreArray(command),                                     //
                SpanOfSize(response.size()),                                   //
                timeout))                                                      //
            .WillOnce(DoAll(FillBuffer<1>(response), Return(expectedResult))); //
    }

    void ExpectRequestAndResponse(                          //
        span<const uint8_t> command,                        //
        span<const uint8_t> response1,                      //
        span<const uint8_t> response2,                      //
        bool expectedResult = true,                         //
        std::chrono::milliseconds timeout = DefaultTimeout) //
    {
        EXPECT_CALL(_lineIOMock,
            ExchangeBuffers(                                                               //
                ElementsAreArray(command),                                                 //
                SpanOfSize(response1.size() + response2.size()),                           //
                timeout))                                                                  //
            .WillOnce(DoAll(FillBuffer<1>(response1, response2), Return(expectedResult))); //
    }

    StrictMock<LineIOMock> _lineIOMock;
    LowLevelCameraDriver _driver;
};

TEST_F(LowLevelCameraTest, ShouldSendAck)
{
    EXPECT_CALL(_lineIOMock, PrintBuffer(ElementsAreArray(commands::AckPackage<CameraCmd::Data, 0x01>))).Times(1);

    _driver.SendAck(CameraCmd::Data, 0x00, 0x01);
}

TEST_F(LowLevelCameraTest, ShouldSendSyncSuccessfully)
{
    ExpectRequestAndResponse(           //
        commands::Sync,                 //
        commands::Ack<CameraCmd::Sync>, //
        commands::Sync);                //

    ASSERT_THAT(_driver.SendSync(DefaultTimeout), Eq(true));
}

TEST_F(LowLevelCameraTest, SyncShouldReturnFalseIfSendFails)
{
    ExpectRequestAndResponse(           //
        commands::Sync,                 //
        commands::Ack<CameraCmd::Sync>, //
        commands::Sync,                 //
        false);                         //

    ASSERT_THAT(_driver.SendSync(DefaultTimeout), Eq(false));
}

TEST_F(LowLevelCameraTest, SyncShouldReturnFalseIfAckInvalid)
{
    ExpectRequestAndResponse(              //
        commands::Sync,                    //
        commands::Ack<CameraCmd::Invalid>, //
        commands::Sync);                   //

    ASSERT_THAT(_driver.SendSync(DefaultTimeout), Eq(false));
}

TEST_F(LowLevelCameraTest, SyncShouldExpectSyncCommandBack)
{
    ExpectRequestAndResponse(           //
        commands::Sync,                 //
        commands::Ack<CameraCmd::Sync>, //
        commands::Invalid);             //

    ASSERT_THAT(_driver.SendSync(DefaultTimeout), Eq(false));
}

TEST_F(LowLevelCameraTest, ShouldSendReset)
{
    ExpectRequestAndResponse(commands::Reset, commands::Ack<CameraCmd::Reset>, true, ResetTimeout);

    ASSERT_THAT(_driver.SendReset(), Eq(true));
}

TEST_F(LowLevelCameraTest, ShouldSendJPEGInitial)
{
    ExpectRequestAndResponse(commands::Init<CameraJPEGResolution::_160x128>, commands::Ack<CameraCmd::Initial>);

    ASSERT_THAT(_driver.SendJPEGInitial(CameraJPEGResolution::_160x128), Eq(true));
}

TEST_F(LowLevelCameraTest, ShouldSendGetPictureJPEG)
{
    ExpectRequestAndResponse(                                                 //
        commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
        commands::Ack<CameraCmd::GetPicture>,                                 //
        commands::Data<CameraPictureType::Enum::Snapshot, 0x01, 0x02, 0x03>); //

    PictureData pictureData;
    ASSERT_THAT(_driver.SendGetPictureJPEG(CameraPictureType::Enum::Snapshot, pictureData), Eq(true));
    ASSERT_THAT(pictureData.type, Eq(CameraPictureType::Enum::Snapshot));
    ASSERT_THAT(pictureData.dataLength, Eq(0x030201U));
}

TEST_F(LowLevelCameraTest, GetPictureShouldFailIfDataResponseIsInvalid)
{
    ExpectRequestAndResponse(                                    //
        commands::GetPicture<CameraPictureType::Enum::Snapshot>, //
        commands::Ack<CameraCmd::GetPicture>,                    //
        commands::Invalid);                                      //

    PictureData pictureData;
    ASSERT_THAT(_driver.SendGetPictureJPEG(CameraPictureType::Enum::Snapshot, pictureData), Eq(false));
}

TEST_F(LowLevelCameraTest, ShouldSendSnapshot)
{
    ExpectRequestAndResponse(                               //
        commands::Snapshot<CameraSnapshotType::Compressed>, //
        commands::Ack<CameraCmd::Snapshot>);                //

    ASSERT_THAT(_driver.SendSnapshot(CameraSnapshotType::Compressed), Eq(true));
}

TEST_F(LowLevelCameraTest, ShouldSendSetPackageSize)
{
    ExpectRequestAndResponse(                      //
        commands::SetPackageSize<0x00, 0x02>,      //
        commands::Ack<CameraCmd::SetPackageSize>); //

    ASSERT_THAT(_driver.SendSetPackageSize(512), Eq(true));
}

TEST_F(LowLevelCameraTest, SetPackageSizeShouldFailIfPackageSizeIsIncorrect)
{
    ASSERT_THAT(_driver.SendSetPackageSize(1024), Eq(false));
}

TEST_F(LowLevelCameraTest, ShouldSendSetBaudRate)
{
    ExpectRequestAndResponse(                   //
        commands::SetBaudRate<0x00, 0x02>,      //
        commands::Ack<CameraCmd::SetBaudRate>); //

    ASSERT_THAT(_driver.SendSetBaudRate(0x00, 0x02), Eq(true));
}
