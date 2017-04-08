#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/PersistentState.hpp"
#include "state/StatePolicies.hpp"

namespace
{
    using testing::Eq;

    class SimpleState
    {
      public:
        SimpleState();

        explicit SimpleState(std::uint32_t newValue);

        bool operator==(const SimpleState& arg) const;

        bool operator!=(const SimpleState& arg) const;

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        static constexpr std::uint32_t Size();

      private:
        std::uint32_t value;
    };

    SimpleState::SimpleState() : value(0)
    {
    }

    SimpleState::SimpleState(std::uint32_t newValue) : value(newValue)
    {
    }

    inline bool SimpleState::operator==(const SimpleState& arg) const
    {
        return this->value == arg.value;
    }

    inline bool SimpleState::operator!=(const SimpleState& arg) const
    {
        return !(*this == arg);
    }

    void SimpleState::Read(Reader& reader)
    {
        this->value = reader.ReadDoubleWordLE();
    }

    void SimpleState::Write(Writer& writer) const
    {
        writer.WriteDoubleWordLE(this->value);
    }

    constexpr std::uint32_t SimpleState::Size()
    {
        return sizeof(std::uint32_t);
    }

    class ComplexState
    {
      public:
        ComplexState();

        ComplexState(std::uint16_t newShort, std::uint8_t newByte);

        bool operator==(const ComplexState& arg) const;

        bool operator!=(const ComplexState& arg) const;

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        static constexpr std::uint32_t Size();

      private:
        std::uint16_t shortValue;
        std::uint8_t byte;
    };

    ComplexState::ComplexState() : shortValue(0), byte(0)
    {
    }

    ComplexState::ComplexState(std::uint16_t newShort, std::uint8_t newByte) : shortValue(newShort), byte(newByte)
    {
    }

    inline bool ComplexState::operator==(const ComplexState& arg) const
    {
        return this->shortValue == arg.shortValue && this->byte == arg.byte;
    }

    inline bool ComplexState::operator!=(const ComplexState& arg) const
    {
        return !(*this == arg);
    }

    void ComplexState::Read(Reader& reader)
    {
        this->shortValue = reader.ReadWordLE();
        this->byte = reader.ReadByte();
    }

    void ComplexState::Write(Writer& writer) const
    {
        writer.WriteWordLE(this->shortValue);
        writer.WriteByte(this->byte);
    }

    constexpr std::uint32_t ComplexState::Size()
    {
        return sizeof(std::uint16_t) + sizeof(std::uint8_t);
    }

    class PersistentStateTest : public testing::Test
    {
      protected:
        state::PersistentState<state::StateTrackingPolicy, SimpleState, ComplexState> state;
    };

    TEST_F(PersistentStateTest, TestDefaultState)
    {
        ASSERT_FALSE(state.IsModified());
        ASSERT_THAT(state.Get<SimpleState>(), Eq(SimpleState()));
        ASSERT_THAT(state.Get<ComplexState>(), Eq(ComplexState()));
    }

    TEST_F(PersistentStateTest, TestSateUpdate)
    {
        state.Set(SimpleState(0x11));
        state.Set(ComplexState(0x22, 0x33));
        ASSERT_THAT(state.Get<SimpleState>(), Eq(SimpleState(0x11)));
        ASSERT_THAT(state.Get<ComplexState>(), Eq(ComplexState(0x22, 0x33)));
    }

    TEST_F(PersistentStateTest, TestSateUpdateMarker)
    {
        state.Set(SimpleState(0x11));
        ASSERT_TRUE(state.IsModified());
    }

    TEST_F(PersistentStateTest, TestSateUpdateMarker2)
    {
        state.Set(ComplexState(0x22, 0x33));
        ASSERT_TRUE(state.IsModified());
    }

    TEST_F(PersistentStateTest, TestStateSize)
    {
        ASSERT_THAT(state.Size(), Eq(SimpleState::Size() + ComplexState::Size()));
    }

    TEST_F(PersistentStateTest, TestStateRead)
    {
        std::uint8_t array[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        Reader reader(gsl::make_span(array));
        state.Read(reader);
        ASSERT_THAT(reader.Status(), Eq(true));
        ASSERT_THAT(state.Get<SimpleState>(), Eq(SimpleState(0X44332211)));
        ASSERT_THAT(state.Get<ComplexState>(), Eq(ComplexState(0x6655, 0x77)));
    }

    TEST_F(PersistentStateTest, TestStateWrite)
    {
        std::uint8_t array[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        std::uint8_t buffer[10];
        state.Set(SimpleState(0x44332211));
        state.Set(ComplexState(0x6655, 0x77));
        Writer writer(gsl::make_span(buffer));
        state.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(array)));
    }

    TEST_F(PersistentStateTest, TestStateCapture)
    {
        std::uint8_t array[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
        std::uint8_t buffer[10];
        state.Set(SimpleState(0x44332211));
        state.Set(ComplexState(0x6655, 0x77));
        Writer writer(gsl::make_span(buffer));
        state.Capture(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(array)));
        ASSERT_FALSE(state.IsModified());
    }
}
