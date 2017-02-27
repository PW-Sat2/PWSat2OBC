#include "FrameContentWriter.hpp"

namespace telecommunication
{
    namespace downlink
    {
        FrameContentWriter::FrameContentWriter(Writer& dataWriter) //
            : writer(dataWriter)
        {
        }
    }
}
