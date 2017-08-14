#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mock/PhotoServiceMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/photo.hpp"

namespace
{
    using testing::Invoke;
    using testing::_;
    using testing::Eq;
    using testing::An;

    class PhotoCommandTest : public testing::Test
    {
      protected:
        PhotoCommandTest();

        TransmitterMock transmitter;
        PhotoServiceMock photo;
        obc::telecommands::TakePhoto command;
    };

    PhotoCommandTest::PhotoCommandTest() : command(photo)
    {
    }

    TEST_F(PhotoCommandTest, TestEmptyParameterList)
    {
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size(), Eq(5));
            EXPECT_THAT(frame[0], Eq(2));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(0));
            EXPECT_THAT(frame[4], Eq(1));
            return true;
        }));
        command.Handle(transmitter, {});
    }

    TEST_F(PhotoCommandTest, TestEmptyPhotoPath)
    {
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size(), Eq(5));
            EXPECT_THAT(frame[0], Eq(2));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(10));
            EXPECT_THAT(frame[4], Eq(2));
            return true;
        }));

        const std::uint8_t array[] = {10, 1, 0, 2};
        command.Handle(transmitter, gsl::make_span(array));
    }

    TEST_F(PhotoCommandTest, TestTooManyPhotos)
    {
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size(), Eq(5));
            EXPECT_THAT(frame[0], Eq(2));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(10));
            EXPECT_THAT(frame[4], Eq(2));
            return true;
        }));

        const std::uint8_t array[] = {10, 1, 0, 30, 'a', 'b', 'c'};
        command.Handle(transmitter, gsl::make_span(array));
    }

    TEST_F(PhotoCommandTest, TestNoPhotos)
    {
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size(), Eq(5));
            EXPECT_THAT(frame[0], Eq(2));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(10));
            EXPECT_THAT(frame[4], Eq(0));
            return true;
        }));

        EXPECT_CALL(photo, Reset()).Times(2);
        EXPECT_CALL(photo, EnableCamera(services::photo::Camera::Wing)).Times(1);
        EXPECT_CALL(photo, DisableCamera(services::photo::Camera::Wing)).Times(1);

        const std::uint8_t array[] = {10, 1, 0, 0, 'a', 'b', 'c'};
        command.Handle(transmitter, gsl::make_span(array));
    }
}
