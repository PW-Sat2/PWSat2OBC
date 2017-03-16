#include "fm25w.hpp"
#include <array>
#include "base/writer.h"
#include "utils.h"

using drivers::spi::SPISelectSlave;
using gsl::make_span;

namespace devices
{
    namespace fm25w
    {
        FM25WDriver::FM25WDriver(drivers::spi::ISPIInterface& spi) : _spi(spi)
        {
        }

        Status FM25WDriver::ReadStatus()
        {
            SPISelectSlave s(this->_spi);

            auto cmd = num(Command::ReadStatusRegister);
            uint8_t status = 0;

            this->_spi.Write(make_span(&cmd, 1));
            this->_spi.Read(make_span(&status, 1));

            return static_cast<Status>(status);
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
    }
}
