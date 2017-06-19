#include "redundancy.hpp"

namespace redundancy
{
    bool CorrectBuffer(gsl::span<std::uint8_t> buffer1, gsl::span<const std::uint8_t> buffer2, gsl::span<const std::uint8_t> buffer3)
    {
        if (buffer1.length() != buffer2.length() || buffer2.length() != buffer3.length())
            return false;

        if (buffer1.length() % 4 != 0)
        {
            return false;
        }

        auto length = buffer1.length() / sizeof(std::uint32_t);

        auto a = reinterpret_cast<std::uint32_t*>(buffer1.data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffer2.data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffer3.data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            a[i] = Correct(a[i], b[i], c[i]);
        }

        return true;
    }

    bool CorrectBuffer(gsl::span<std::uint8_t> result, const std::array<gsl::span<const std::uint8_t>, 5>& buffers)
    {
        if (buffers[0].length() != buffers[1].length() || buffers[1].length() != buffers[2].length() ||
            buffers[2].length() != buffers[3].length() || buffers[3].length() != buffers[4].length() ||
            result.length() != buffers[0].length())
            return false;

        if (result.length() % 4 != 0)
        {
            return false;
        }

        auto length = result.length() / sizeof(std::uint32_t);

        auto r = reinterpret_cast<std::uint32_t*>(result.data());

        auto a = reinterpret_cast<const std::uint32_t*>(buffers[0].data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffers[1].data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffers[2].data());
        auto d = reinterpret_cast<const std::uint32_t*>(buffers[3].data());
        auto e = reinterpret_cast<const std::uint32_t*>(buffers[4].data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c) && IsAligned<4>(d) && IsAligned<4>(e)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            r[i] = Correct(a[i], b[i], c[i], d[i], e[i]);
        }

        return true;
    }

    bool CorrectBuffer(gsl::span<std::uint8_t> output,
        gsl::span<const std::uint8_t> buffer1,
        gsl::span<const std::uint8_t> buffer2,
        gsl::span<const std::uint8_t> buffer3)
    {
        if (output.length() != buffer1.length() || buffer1.length() != buffer2.length() || buffer2.length() != buffer3.length())
            return false;

        if (output.length() % 4 != 0)
        {
            return false;
        }

        auto length = output.length() / sizeof(std::uint32_t);

        auto r = reinterpret_cast<std::uint32_t*>(output.data());

        auto a = reinterpret_cast<const std::uint32_t*>(buffer1.data());
        auto b = reinterpret_cast<const std::uint32_t*>(buffer2.data());
        auto c = reinterpret_cast<const std::uint32_t*>(buffer3.data());

        if (!(IsAligned<4>(a) && IsAligned<4>(b) && IsAligned<4>(c)))
        {
            return false;
        }

        for (decltype(length) i = 0; i < length; ++i)
        {
            r[i] = Correct(a[i], b[i], c[i]);
        }

        return true;
    }
}
