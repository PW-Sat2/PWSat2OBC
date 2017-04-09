#include <string.h>
#include <algorithm>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mock/StorageAccessMock.hpp"
#include "obc/ObcState.hpp"
#include "state/struct.h"

using testing::Invoke;
using testing::Eq;
using testing::_;

using namespace std::chrono_literals;

class ObcStateTest : public testing::Test
{
  protected:
    StorageAccessMock storage;
    state::SystemPersistentState stateObject;
};

TEST_F(ObcStateTest, TestWritingDefaultState)
{
    EXPECT_CALL(storage, Write(4, _)).WillOnce(Invoke([](std::uint32_t, gsl::span<const std::uint8_t> buffer) {
        const std::uint8_t tab[] = {0xee,
            0x77,
            0xaa,
            0x55,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0xee,
            0x77,
            0xaa,
            0x55};
        ASSERT_THAT(gsl::make_span(tab), Eq(buffer));
    }));

    obc::WritePersistentState(this->stateObject, 4, this->storage);
}

TEST_F(ObcStateTest, TestWritingCustomState)
{
    stateObject.Set(state::AntennaConfiguration(true));
    stateObject.Set(state::TimeState(8603657889541918976ms, 8690164679560785663ms));
    EXPECT_CALL(storage, Write(8, _)).WillOnce(Invoke([](std::uint32_t, gsl::span<const std::uint8_t> buffer) {
        const std::uint8_t tab[] = {0xee,
            0x77,
            0xaa,
            0x55,
            0x1,
            0x00,
            0x11,
            0x22,
            0x33,
            0x44,
            0x55,
            0x66,
            0x77,
            0xff,
            0xee,
            0xdd,
            0xcc,
            0xbb,
            0xaa,
            0x99,
            0x78,
            0xee,
            0x77,
            0xaa,
            0x55};
        ASSERT_THAT(gsl::make_span(tab), Eq(buffer));
    }));

    obc::WritePersistentState(this->stateObject, 8, this->storage);
}

TEST_F(ObcStateTest, TestReadingStateInvalidForwardSignagure)
{
    EXPECT_CALL(storage, Read(8, _)).WillOnce(Invoke([](std::uint32_t, gsl::span<std::uint8_t> buffer) {
        memset(buffer.data(), 0x00, buffer.size());
        const auto size = buffer.size();
        buffer[size - 4u] = 0xee;
        buffer[size - 3u] = 0x77;
        buffer[size - 2u] = 0xaa;
        buffer[size - 1u] = 0x55;
    }));

    ASSERT_FALSE(obc::ReadPersistentState(this->stateObject, 8, this->storage));
}

TEST_F(ObcStateTest, TestReadingStateInvalidEndSignature)
{
    EXPECT_CALL(storage, Read(8, _)).WillOnce(Invoke([](std::uint32_t, gsl::span<std::uint8_t> buffer) {
        memset(buffer.data(), 0x00, buffer.size());
        buffer[0] = 0xee;
        buffer[1] = 0x77;
        buffer[2] = 0xaa;
        buffer[3] = 0x55;
    }));

    ASSERT_FALSE(obc::ReadPersistentState(this->stateObject, 8, this->storage));
}

TEST_F(ObcStateTest, TestReadingState)
{
    EXPECT_CALL(storage, Read(8, _)).WillOnce(Invoke([](std::uint32_t, gsl::span<std::uint8_t> buffer) {
        const std::uint8_t tab[] = {0xee,
            0x77,
            0xaa,
            0x55,
            0x1,
            0x00,
            0x11,
            0x22,
            0x33,
            0x44,
            0x55,
            0x66,
            0x77,
            0xff,
            0xee,
            0xdd,
            0xcc,
            0xbb,
            0xaa,
            0x99,
            0x78,
            0xee,
            0x77,
            0xaa,
            0x55};
        memcpy(buffer.data(), tab, std::min<size_t>(buffer.size(), sizeof(tab)));
    }));

    ASSERT_TRUE(obc::ReadPersistentState(this->stateObject, 8, this->storage));

    ASSERT_THAT(stateObject.Get<state::AntennaConfiguration>(), Eq(state::AntennaConfiguration(true)));
    ASSERT_THAT(stateObject.Get<state::TimeState>(), Eq(state::TimeState(8603657889541918976ms, 8690164679560785663ms)));
}
