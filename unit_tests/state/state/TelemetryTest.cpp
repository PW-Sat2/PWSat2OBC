#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"
#include "state/Telemetry.hpp"

namespace
{
    using testing::Eq;

    class SimpleObject
    {
      public:
        static constexpr std::uint32_t Id = 5;

        SimpleObject();

        explicit SimpleObject(std::uint32_t newValue);

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        bool IsDifferent(const SimpleObject& arg) const;

        static constexpr std::uint32_t Size();

        std::uint32_t Value() const;

      private:
        std::uint32_t value;
    };

    SimpleObject::SimpleObject() : value(0)
    {
    }

    SimpleObject::SimpleObject(std::uint32_t newValue) : value(newValue)
    {
    }

    void SimpleObject::Read(Reader& reader)
    {
        this->value = reader.ReadDoubleWordLE();
    }

    void SimpleObject::Write(Writer& writer) const
    {
        writer.WriteDoubleWordLE(this->value);
    }

    inline bool SimpleObject::IsDifferent(const SimpleObject& arg) const
    {
        return this->value != arg.value;
    }

    constexpr std::uint32_t SimpleObject::Size()
    {
        return sizeof(std::uint32_t);
    }

    inline std::uint32_t SimpleObject::Value() const
    {
        return this->value;
    }

    class ComplexObject
    {
      public:
        static constexpr std::uint32_t Id = 7;

        ComplexObject();

        ComplexObject(std::uint16_t newShort, std::uint8_t newByte);

        void Read(Reader& reader);

        void Write(Writer& writer) const;

        bool IsDifferent(const ComplexObject& arg) const;

        static constexpr std::uint32_t Size();

        std::uint16_t ShortValue() const;

        std::uint8_t Byte() const;

      private:
        std::uint16_t shortValue;
        std::uint8_t byte;
    };

    ComplexObject::ComplexObject() : shortValue(0), byte(0)
    {
    }

    ComplexObject::ComplexObject(std::uint16_t newShort, std::uint8_t newByte) : shortValue(newShort), byte(newByte)
    {
    }

    void ComplexObject::Read(Reader& reader)
    {
        this->shortValue = reader.ReadWordLE();
        this->byte = reader.ReadByte();
    }

    void ComplexObject::Write(Writer& writer) const
    {
        writer.WriteWordLE(this->shortValue);
        writer.WriteByte(this->byte);
    }

    inline bool ComplexObject::IsDifferent(const ComplexObject& arg) const
    {
        return std::abs(this->shortValue - arg.shortValue) >= 10 && std::abs(this->byte - arg.byte) >= 5;
    }

    constexpr std::uint32_t ComplexObject::Size()
    {
        return sizeof(std::uint16_t) + sizeof(std::uint8_t);
    }

    inline std::uint16_t ComplexObject::ShortValue() const
    {
        return this->shortValue;
    }

    inline std::uint8_t ComplexObject::Byte() const
    {
        return this->byte;
    }

    typedef state::Telemetry<SimpleObject, ComplexObject> Telemetry;

    class TelemetryTest : public testing::Test
    {
      protected:
        Telemetry telemetry;
    };

    TEST_F(TelemetryTest, TestTypeCount)
    {
        ASSERT_THAT(Telemetry::TypeCount, Eq(2));
    }

    TEST_F(TelemetryTest, TestPayloadSize)
    {
        ASSERT_THAT(Telemetry::PayloadSize, Eq(4 + 3));
    }

    TEST_F(TelemetryTest, TestTotalSerializedSize)
    {
        ASSERT_THAT(Telemetry::TotalSerializedSize, Eq(1 + 4 + 1 + 1 + 3 + 1));
    }

    TEST_F(TelemetryTest, TestIsModifiedDefaultState)
    {
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestDefaultState)
    {
        const auto& simple = telemetry.Get<SimpleObject>();
        ASSERT_THAT(simple.Value(), Eq(0u));
        const auto& complex = telemetry.Get<ComplexObject>();
        ASSERT_THAT(complex.ShortValue(), Eq(0u));
        ASSERT_THAT(complex.Byte(), Eq(0u));
    }

    TEST_F(TelemetryTest, TestSetingValueSimpleCase)
    {
        telemetry.Set(SimpleObject(1));
        const auto& simple = telemetry.Get<SimpleObject>();
        ASSERT_THAT(simple.Value(), Eq(1u));
    }

    TEST_F(TelemetryTest, TestSetingValueUpdatesModificationFlat)
    {
        telemetry.Set(SimpleObject(1));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestCommitCaptureClearsModificationFlag)
    {
        telemetry.Set(SimpleObject(1));
        telemetry.CommitCapture();
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestSettingTheSameValueDoesNotSetModificationFlag)
    {
        telemetry.Set(SimpleObject(1));
        telemetry.CommitCapture();
        telemetry.Set(SimpleObject(1));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestSettingVolatileDoesNotSetModificationFlag)
    {
        telemetry.SetVolatile(SimpleObject(1));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestWriteModifiedSavesNothingWhenThereAreNoChanges)
    {
        std::array<std::uint8_t, Telemetry::TotalSerializedSize> buffer;
        Writer writer(buffer);
        telemetry.SetVolatile(SimpleObject(1));
        telemetry.WriteModified(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture().empty(), Eq(true));
    }

    TEST_F(TelemetryTest, TestWriteModifiedSavesChanges)
    {
        std::array<std::uint8_t, Telemetry::TotalSerializedSize> buffer;
        std::uint8_t expected[] = {0x5, 0x1, 0x0, 0x0, 0x0, 0x5};
        Writer writer(buffer);
        telemetry.Set(SimpleObject(1));
        telemetry.WriteModified(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        auto span = writer.Capture();
        ASSERT_THAT(span, Eq(gsl::make_span(expected)));
    }

    TEST_F(TelemetryTest, TestMinorChangesAreIgnoredUp)
    {
        telemetry.Set(ComplexObject(9, 4));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestMinorChangesAreIgnoredDown)
    {
        telemetry.Set(ComplexObject(20, 30));
        telemetry.CommitCapture();
        telemetry.Set(ComplexObject(11, 26));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestSequenceOfMinorChangesAccumulatesToMajorChange)
    {
        telemetry.Set(ComplexObject(20, 30));
        telemetry.CommitCapture();
        telemetry.Set(ComplexObject(15, 26));
        telemetry.Set(ComplexObject(9, 22));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestFullSerialization)
    {
        std::array<std::uint8_t, Telemetry::TotalSerializedSize> buffer;
        std::uint8_t expected[] = {0x5, 0xee, 0x77, 0xaa, 0x55, 0x5, 0x7, 0x0f, 0x00, 0x1a, 0x7};
        Writer writer(buffer);
        telemetry.Set(ComplexObject(15, 26));
        telemetry.Set(SimpleObject(0x55aa77ee));
        telemetry.WriteModified(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        auto span = writer.Capture();
        ASSERT_THAT(span, Eq(gsl::make_span(expected)));
    }

    TEST_F(TelemetryTest, TestContainerInterfaceSet)
    {
        ITelemetryContainer<SimpleObject>* ptr = &telemetry;
        ptr->Set(SimpleObject(1));
        const auto& simple = ptr->Get();
        ASSERT_THAT(simple.Value(), Eq(1u));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestContainerInterfaceSetVolatile)
    {
        ITelemetryContainer<SimpleObject>* ptr = &telemetry;
        ptr->SetVolatile(SimpleObject(1));
        const auto& simple = ptr->Get();
        ASSERT_THAT(simple.Value(), Eq(1u));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestContainerInterfaceSetMinorChange)
    {
        ITelemetryContainer<ComplexObject>* ptr = &telemetry;
        ptr->Set(ComplexObject(9, 4));
        const auto& complex = ptr->Get();
        ASSERT_THAT(complex.ShortValue(), Eq(0u));
        ASSERT_THAT(complex.Byte(), Eq(0u));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }
}
