#include "Frame.hpp"

COMM_BEGIN

Frame::Frame() : doppler(0), rssi(0), fullFrameSize(0)
{
}

Frame::Frame(std::uint16_t dopplerLevel, std::uint16_t rssiLevel, std::uint16_t fullSize, gsl::span<std::uint8_t> data)
    : doppler(dopplerLevel),   //
      rssi(rssiLevel),         //
      fullFrameSize(fullSize), //
      content(std::move(data))
{
}

COMM_END
