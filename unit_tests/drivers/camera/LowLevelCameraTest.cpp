#include <array>

#include <gsl/span>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/LineIOMock.hpp"
#include "utils.hpp"

#include "CameraTestCommands.hpp"
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

static constexpr std::chrono::milliseconds DefaultTimeout = 5s;
static constexpr std::chrono::milliseconds ResetTimeout = std::chrono::milliseconds(100);

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
