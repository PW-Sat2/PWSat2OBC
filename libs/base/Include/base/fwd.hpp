#ifndef LIBS_BASE_FWD_HPP
#define LIBS_BASE_FWD_HPP

#pragma once

#include <cstdint>

class Reader;
class Writer;

template <typename Type> struct ITelemetryContainer;
template <typename Underlying, std::uint8_t BitsCount> class BitValue;

#endif
