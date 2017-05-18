#include "fm25w.hpp"
#include <array>
#include "base/writer.h"
#include "redundancy.hpp"
#include "utils.h"

using drivers::spi::SPISelectSlave;
using gsl::make_span;

using redundancy::Vote;
using redundancy::CorrectBuffer;

namespace devices
{
    namespace fm25w
    {
        FM25WDriver::FM25WDriver(drivers::spi::ISPIInterface& spi) : _spi(spi)
        {
        }

        Option<Status> FM25WDriver::ReadStatus()
        {
            SPISelectSlave s(this->_spi);

            auto cmd = num(Command::ReadStatusRegister);
            uint8_t status = 0;

            this->_spi.Write(make_span(&cmd, 1));
            this->_spi.Read(make_span(&status, 1));

            return Some(static_cast<Status>(status));
        }

        void FM25WDriver::Read(Address address, gsl::span<std::uint8_t> buffer)
        {
            std::array<uint8_t, 3> cmd;
            Writer w(cmd);
            w.WriteByte(num(Command::Read));
            w.WriteWordBE(address);

            SPISelectSlave s(this->_spi);
            this->_spi.Write(cmd);
            this->_spi.Read(buffer);
        }

        void FM25WDriver::Write(Address address, gsl::span<const std::uint8_t> buffer)
        {
            std::array<uint8_t, 3> cmd;
            Writer w(cmd);
            w.WriteByte(num(Command::Write));
            w.WriteWordBE(address);

            {
                SPISelectSlave s(this->_spi);
                auto cmd = num(Command::EnableWrite);
                this->_spi.Write(make_span(&cmd, 1));
            }

            {
                SPISelectSlave s(this->_spi);
                this->_spi.Write(cmd);
                this->_spi.Write(buffer);
            }
        }

        RedundantFM25WDriver::RedundantFM25WDriver(std::array<IFM25WDriver*, 3> fm25wDrivers) : _fm25wDrivers(fm25wDrivers)
        {
        }

        Option<Status> RedundantFM25WDriver::ReadStatus()
        {
            auto status1 = _fm25wDrivers[0]->ReadStatus();
            auto status2 = _fm25wDrivers[1]->ReadStatus();
            auto status3 = _fm25wDrivers[2]->ReadStatus();

            return Vote(status1.Value, status2.Value, status3.Value);
        }

        void RedundantFM25WDriver::Read(Address address, gsl::span<uint8_t> outputBuffer)
        {
            std::array<uint8_t, 1_KB> redundantBuffer1;
            std::array<uint8_t, 1_KB> redundantBuffer2;

            for (ptrdiff_t offset = 0; offset < outputBuffer.length(); offset += 1_KB)
            {
                auto length = std::min(static_cast<size_t>(outputBuffer.length() - offset), 1_KB);
                auto outputSpan = outputBuffer.subspan(offset, length);

                Read(address + offset, outputSpan, redundantBuffer1, redundantBuffer2);
            }
        }

        void RedundantFM25WDriver::Read(Address address,
            gsl::span<uint8_t> outputBuffer,     //
            gsl::span<uint8_t> redundantBuffer1, //
            gsl::span<uint8_t> redundantBuffer2)
        {
            auto bufferLength = std::min(outputBuffer.length(), std::min(redundantBuffer1.length(), redundantBuffer2.length()));

            auto normalizedOutputBuffer = outputBuffer.subspan(0, bufferLength);
            auto normalizedRedundantBuffer1 = redundantBuffer1.subspan(0, bufferLength);

            _fm25wDrivers[0]->Read(address, normalizedOutputBuffer);
            _fm25wDrivers[1]->Read(address, normalizedRedundantBuffer1);

            if (normalizedOutputBuffer == normalizedRedundantBuffer1)
            {
                return;
            }

            {
                auto normalizedRedundantBuffer2 = redundantBuffer2.subspan(0, bufferLength);

                _fm25wDrivers[2]->Read(address, normalizedRedundantBuffer2);
                CorrectBuffer(normalizedOutputBuffer, normalizedRedundantBuffer1, normalizedRedundantBuffer2);
            }
        }

        void RedundantFM25WDriver::Write(Address address, gsl::span<const std::uint8_t> buffer)
        {
            _fm25wDrivers[0]->Write(address, buffer);
            _fm25wDrivers[1]->Write(address, buffer);
            _fm25wDrivers[2]->Write(address, buffer);
        }
    }
}
