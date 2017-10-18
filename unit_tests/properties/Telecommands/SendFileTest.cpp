#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "obc/telecommands/file_system.hpp"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

namespace
{
    RC_GTEST_PROP(SendFileTest, MaxSeqNumberMustBeValid, (std::uint32_t size))
    {
        auto maxChunk = obc::telecommands::FileSender::MaxChunkNumber(size);

        RC_ASSERT(maxChunk * 230 >= size);
        RC_ASSERT(maxChunk * 230 < size + 230);
    }
}
