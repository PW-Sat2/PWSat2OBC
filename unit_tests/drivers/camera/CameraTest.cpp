#include <array>

#include <gsl/span>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock/LineIOMock.hpp"
#include "utils.hpp"

#include "CameraTestCommands.hpp"
#include "camera/camera.h"

using std::array;
using gsl::span;

using testing::Test;
using testing::Eq;
using testing::StrictMock;
using testing::ElementsAreArray;
using testing::Return;
using testing::DoAll;
using testing::_;
using testing::InSequence;

using namespace devices::camera;
using namespace std::chrono_literals;

class CameraTest : public Test
{
  public:
    CameraTest() : _camera(_lineIOMock)
    {
    }

    void ExpectRequestAndResponse(          //
        span<const uint8_t> command,        //
        span<const uint8_t> response,       //
        bool expectedResult = true,         //
        uint8_t expectationCardinality = 1) //
    {
        EXPECT_CALL(_lineIOMock,
            ExchangeBuffers(                 //
                ElementsAreArray(command),   //
                SpanOfSize(response.size()), //
                _))
            .Times(expectationCardinality)
            .WillRepeatedly(DoAll(FillBuffer<1>(response), Return(expectedResult))); //
    }

    void ExpectRequestAndResponse(          //
        span<const uint8_t> command,        //
        span<const uint8_t> response1,      //
        span<const uint8_t> response2,      //
        bool expectedResult = true,         //
        uint8_t expectationCardinality = 1) //
    {
        EXPECT_CALL(_lineIOMock,
            ExchangeBuffers(                                                                     //
                ElementsAreArray(command),                                                       //
                SpanOfSize(response1.size() + response2.size()),                                 //
                _))                                                                              //
            .Times(expectationCardinality)                                                       //
            .WillRepeatedly(DoAll(FillBuffer<1>(response1, response2), Return(expectedResult))); //
    }

    void ExpectRequest(span<const uint8_t> command)
    {
        EXPECT_CALL(_lineIOMock, PrintBuffer(ElementsAreArray(command))).Times(1);
    }

    StrictMock<LineIOMock> _lineIOMock;
    Camera _camera;
};

TEST_F(CameraTest, InitializeWithIdealSync)
{
    {
        InSequence s;

        ExpectRequestAndResponse(commands::Reset, commands::Ack<CameraCmd::Reset>);

        ExpectRequestAndResponse(           //
            commands::Sync,                 //
            commands::Ack<CameraCmd::Sync>, //
            commands::Sync);                //

        ExpectRequest(commands::Ack<CameraCmd::Sync>);
    }

    auto result = _camera.Initialize();
    ASSERT_THAT(result.IsSuccess, Eq(true));
    ASSERT_THAT(result.SyncCount, Eq(1));
}

TEST_F(CameraTest, InitializeWithNormalSync)
{
    {
        InSequence s;

        ExpectRequestAndResponse(commands::Reset, commands::Ack<CameraCmd::Reset>);

        ExpectRequestAndResponse( //
            commands::Sync,       //
            commands::Invalid,    //
            commands::Invalid,    //
            true,                 //
            10);                  //

        ExpectRequestAndResponse(           //
            commands::Sync,                 //
            commands::Ack<CameraCmd::Sync>, //
            commands::Sync);

        ExpectRequest(commands::Ack<CameraCmd::Sync>);
    }

    auto result = _camera.Initialize();
    ASSERT_THAT(result.IsSuccess, Eq(true));
    ASSERT_THAT(result.SyncCount, Eq(11));
}

TEST_F(CameraTest, FailToSync)
{
    {
        InSequence s;

        ExpectRequestAndResponse(commands::Reset, commands::Ack<CameraCmd::Reset>);

        ExpectRequestAndResponse( //
            commands::Sync,       //
            commands::Invalid,    //
            commands::Invalid,    //
            true,                 //
            60);
    }

    auto result = _camera.Initialize();
    ASSERT_THAT(result.IsSuccess, Eq(false));
    ASSERT_THAT(result.SyncCount, Eq(60));
}

TEST_F(CameraTest, TakeJPEGPictureSuccessfully)
{
    {
        InSequence s;

        ExpectRequestAndResponse(                           //
            commands::Init<CameraJPEGResolution::_320x240>, //
            commands::Ack<CameraCmd::Initial>);             //

        ExpectRequestAndResponse(                      //
            commands::SetPackageSize<0x00, 0x02>,      //
            commands::Ack<CameraCmd::SetPackageSize>); //

        ExpectRequestAndResponse(                               //
            commands::Snapshot<CameraSnapshotType::Compressed>, //
            commands::Ack<CameraCmd::Snapshot>);                //
    }

    auto result = _camera.TakeJPEGPicture(CameraJPEGResolution::_320x240);

    ASSERT_THAT(result, Eq(true));
}

TEST_F(CameraTest, TestTakeJPEGPictureIfInitFails)
{
    {
        InSequence s;

        ExpectRequestAndResponse(                           //
            commands::Init<CameraJPEGResolution::_320x240>, //
            commands::Ack<CameraCmd::Invalid>);             //
    }

    auto result = _camera.TakeJPEGPicture(CameraJPEGResolution::_320x240);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(CameraTest, TestTakeJPEGPictureIfSetPackageSizeFails)
{
    {
        InSequence s;

        ExpectRequestAndResponse(                           //
            commands::Init<CameraJPEGResolution::_320x240>, //
            commands::Ack<CameraCmd::Initial>);             //

        ExpectRequestAndResponse(                 //
            commands::SetPackageSize<0x00, 0x02>, //
            commands::Invalid);                   //
    }

    auto result = _camera.TakeJPEGPicture(CameraJPEGResolution::_320x240);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(CameraTest, TestTakeJPEGPictureIfSnapshotFails)
{
    {
        InSequence s;

        ExpectRequestAndResponse(                           //
            commands::Init<CameraJPEGResolution::_320x240>, //
            commands::Ack<CameraCmd::Initial>);             //

        ExpectRequestAndResponse(                      //
            commands::SetPackageSize<0x00, 0x02>,      //
            commands::Ack<CameraCmd::SetPackageSize>); //

        ExpectRequestAndResponse(                               //
            commands::Snapshot<CameraSnapshotType::Compressed>, //
            commands::Invalid);                                 //
    }

    auto result = _camera.TakeJPEGPicture(CameraJPEGResolution::_320x240);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(CameraTest, TestRetrievingSinglePackagePicture)
{
    std::array<uint8_t, 512> cameraImageBuffer;
    cameraImageBuffer.fill(0xD0);

    std::array<uint8_t, 512> receiveBuffer;

    {
        InSequence s;

        ExpectRequestAndResponse(                                                 //
            commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
            commands::Ack<CameraCmd::GetPicture>,                                 //
            commands::Data<CameraPictureType::Enum::Snapshot, 0xFA, 0x01, 0x00>); // 512-6

        ExpectRequestAndResponse(commands::Ack<CameraCmd::None>, cameraImageBuffer);

        ExpectRequest(commands::AckPackage<CameraCmd::None, 0xF0, 0xF0>);
    }

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(512));
}

TEST_F(CameraTest, TestRetrievingMultiplePackagesPicture)
{
    std::array<uint8_t, 512> cameraImageBuffer;
    cameraImageBuffer.fill(0xD0);

    std::array<uint8_t, 1024> receiveBuffer;

    {
        InSequence s;

        ExpectRequestAndResponse(                                                 //
            commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
            commands::Ack<CameraCmd::GetPicture>,                                 //
            commands::Data<CameraPictureType::Enum::Snapshot, 0xF4, 0x03, 0x00>); // 1024-12

        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x00, 0x00>, cameraImageBuffer);
        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x01, 0x00>, cameraImageBuffer);

        ExpectRequest(commands::AckPackage<CameraCmd::None, 0xF0, 0xF0>);
    }

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(1024));
}

TEST_F(CameraTest, TestRetrievingPictureNotFittingIntoPackages)
{
    std::array<uint8_t, 512> cameraImageBuffer;
    cameraImageBuffer.fill(0xD0);

    std::array<uint8_t, 1024> receiveBuffer;

    {
        InSequence s;

        ExpectRequestAndResponse(                                                 //
            commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
            commands::Ack<CameraCmd::GetPicture>,                                 //
            commands::Data<CameraPictureType::Enum::Snapshot, 0xF4, 0x03, 0x00>); // 506*1.5

        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x00, 0x00>, cameraImageBuffer);
        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x01, 0x00>, cameraImageBuffer);

        ExpectRequest(commands::AckPackage<CameraCmd::None, 0xF0, 0xF0>);
    }

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(0x3F4 + 12));
}

TEST_F(CameraTest, TestRetrievingPictureIfGetPictureFails)
{
    std::array<uint8_t, 1024> receiveBuffer;

    ExpectRequestAndResponse(                                    //
        commands::GetPicture<CameraPictureType::Enum::Snapshot>, //
        commands::Invalid,                                       //
        commands::Invalid);                                      //

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(CameraTest, TestRetrievingPictureIfBufferIsToSmall)
{
    std::array<uint8_t, 128> receiveBuffer;

    ExpectRequestAndResponse(                                                 //
        commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
        commands::Ack<CameraCmd::GetPicture>,                                 //
        commands::Data<CameraPictureType::Enum::Snapshot, 0x00, 0x02, 0x00>); // 1024

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(0));
}

TEST_F(CameraTest, TestRetrievingPartialPicture)
{
    std::array<uint8_t, 512> cameraImageBuffer;
    cameraImageBuffer.fill(0xD0);

    std::array<uint8_t, 2048> receiveBuffer;

    {
        InSequence s;

        ExpectRequestAndResponse(                                                 //
            commands::GetPicture<CameraPictureType::Enum::Snapshot>,              //
            commands::Ack<CameraCmd::GetPicture>,                                 //
            commands::Data<CameraPictureType::Enum::Snapshot, 0xF4, 0x03, 0x00>); // 506*1.5

        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x00, 0x00>, cameraImageBuffer);
        ExpectRequestAndResponse(commands::AckPackage<CameraCmd::None, 0x01, 0x00>, cameraImageBuffer, false);

        ExpectRequest(commands::AckPackage<CameraCmd::None, 0xF0, 0xF0>);
    }

    auto result = _camera.CameraReceiveJPEGData(receiveBuffer);
    ASSERT_THAT(result.size(), Eq(512));
}
