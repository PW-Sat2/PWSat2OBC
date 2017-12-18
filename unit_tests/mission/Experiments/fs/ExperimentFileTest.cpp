#include <gsl/span>
#include <iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "fs/ExperimentFile.hpp"
#include "mock/FsMock.hpp"
#include "mock/time.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;
using testing::_;

using namespace experiments::fs;
using namespace std::chrono_literals;
using namespace services::fs;

namespace
{
    class ExperimentFileTest : public testing::Test
    {
      public:
        ExperimentFileTest();

        const char* TestFileName = "/test";

      protected:
        NiceMock<FsMock> _fs;
        NiceMock<CurrentTimeMock> _time;
        std::array<uint8_t, 2 * ExperimentFile::PacketLength> _buffer;
    };

    ExperimentFileTest::ExperimentFileTest()
    {
        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(65536ms)));
        _buffer.fill(0xFF);
        this->_fs.AddFile(TestFileName, _buffer);
    }

    TEST_F(ExperimentFileTest, EmptyFileWithTimestamp)
    {
        ExperimentFile file(&_time);
        uint8_t data = 7;
        file.Open(_fs, TestFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
        file.Write(ExperimentFile::PID::Reserved, gsl::make_span(&data, 1));
        file.Close();

        std::array<uint8_t, 2 * ExperimentFile::PacketLength> expected;
        expected.fill(0xFF);
        Writer w(expected);

        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);
        w.WriteByte(num(ExperimentFile::PID::Reserved));
        w.WriteByte(7);

        ASSERT_THAT(_buffer, Eq(expected));
    }

    TEST_F(ExperimentFileTest, EmptyFileWithoutTimestamp)
    {
        ExperimentFile file;
        uint8_t data = 7;
        file.Open(_fs, TestFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
        file.Write(ExperimentFile::PID::Reserved, gsl::make_span(&data, 1));
        file.Close();

        std::array<uint8_t, 2 * ExperimentFile::PacketLength> expected;
        expected.fill(0xFF);
        Writer w(expected);

        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Reserved));
        w.WriteByte(7);

        ASSERT_THAT(_buffer, Eq(expected));
    }

    TEST_F(ExperimentFileTest, WriteSmallDataPacketsAndCheckPacketPadding)
    {
        ExperimentFile file(&_time);
        file.Open(_fs, TestFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

        std::array<uint8_t, 9> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        for (int i = 0; i < 21; ++i)
        {
            file.Write(ExperimentFile::PID::Reserved, data);
        }

        file.Close();

        std::array<uint8_t, 2 * ExperimentFile::PacketLength> expected;
        expected.fill(0xFF);
        Writer w(expected);

        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);
        for (int i = 0; i < 21; ++i)
        {
            w.WriteByte(num(ExperimentFile::PID::Reserved));
            w.WriteArray(data);
        }

        ASSERT_THAT(_buffer, Eq(expected));
    }

    TEST_F(ExperimentFileTest, WriteSmallDataPacketsAndCheckPacketSplitting)
    {
        ExperimentFile file(&_time);
        file.Open(_fs, TestFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

        std::array<uint8_t, 9> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        for (int i = 0; i < 21; ++i)
        {
            file.Write(ExperimentFile::PID::Reserved, data);
        }

        file.Write(ExperimentFile::PID::Reserved, gsl::make_span(data).subspan(0, 8));

        file.Write(ExperimentFile::PID::Reserved, data);

        file.Close();

        std::array<uint8_t, 2 * ExperimentFile::PacketLength> expected;
        expected.fill(0xFF);
        Writer w(expected);

        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);
        for (int i = 0; i < 21; ++i)
        {
            w.WriteByte(num(ExperimentFile::PID::Reserved));
            w.WriteArray(data);
        }

        w.WriteByte(num(ExperimentFile::PID::Reserved));
        w.WriteArray(gsl::make_span(data).subspan(0, 8));

        w.WriteByte(0xFF);

        // second packet
        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);
        w.WriteByte(num(ExperimentFile::PID::Reserved));
        w.WriteArray(data);

        for (auto i = 0U; i < expected.size(); i++)
        {
            std::cout << i << "\t"
                      << "A = " << (int)_buffer[i] << "\t"
                      << "E = " << (int)expected[i] << std::endl;
        }

        ASSERT_THAT(_buffer, Eq(expected));
    }

    TEST_F(ExperimentFileTest, WriteBigDataAndCheckPacketSplitting)
    {
        ExperimentFile file(&_time);
        file.Open(_fs, TestFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

        std::array<uint8_t, 300> data;
        data.fill(0xBC);
        file.Write(ExperimentFile::PID::Reserved, data);

        file.Close();

        std::array<uint8_t, 2 * ExperimentFile::PacketLength> expected;
        expected.fill(0xFF);
        Writer w(expected);

        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);

        w.WriteByte(num(ExperimentFile::PID::Reserved));

        auto length = ExperimentFile::PacketLength - 3 - sizeof(uint64_t);

        for (uint32_t i = 0; i < length; ++i)
        {
            w.WriteByte(0xBC);
        }

        // second packet
        w.WriteByte(num(ExperimentFile::PID::Synchronization));
        w.WriteByte(num(ExperimentFile::PID::Timestamp));
        w.WriteQuadWordLE(65536);
        w.WriteByte(num(ExperimentFile::PID::Continuation));
        w.WriteByte(num(ExperimentFile::PID::Reserved));
        auto secondPacketLength = data.size() - length;
        for (uint32_t i = 0; i < secondPacketLength; ++i)
        {
            w.WriteByte(0xBC);
        }

        ASSERT_THAT(_buffer, Eq(expected));
    }
}
