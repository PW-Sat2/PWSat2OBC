#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/Telemetry.hpp"

namespace
{
    using testing::Eq;

    class SimpleObject
    {
      public:
        static constexpr std::uint32_t Id = 5;

        SimpleObject();

        explicit SimpleObject(std::uint32_t newValue);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

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
    void SimpleObject::Write(BitWriter& writer) const
    {
        writer.Write(this->value);
    }

    constexpr std::uint32_t SimpleObject::BitSize()
    {
        return sizeof(std::uint32_t) * 8;
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

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

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

    void ComplexObject::Write(BitWriter& writer) const
    {
        writer.Write(this->shortValue);
        writer.Write(this->byte);
    }

    constexpr std::uint32_t ComplexObject::BitSize()
    {
        return sizeof(std::uint16_t) * 8 + sizeof(std::uint8_t) * 8;
    }

    inline std::uint16_t ComplexObject::ShortValue() const
    {
        return this->shortValue;
    }

    inline std::uint8_t ComplexObject::Byte() const
    {
        return this->byte;
    }

    typedef telemetry::Telemetry<SimpleObject, ComplexObject> Telemetry;

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
        ASSERT_THAT(Telemetry::PayloadSize, Eq(8 * (4 + 3)));
    }

    TEST_F(TelemetryTest, TestTotalSerializedSize)
    {
        ASSERT_THAT(Telemetry::TotalSerializedSize, Eq(4 + 3));
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

    TEST_F(TelemetryTest, TestSettingTheSame)
    {
        telemetry.Set(SimpleObject(1));
        telemetry.CommitCapture();
        telemetry.Set(SimpleObject(1));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestSettingVolatileDoesNotSetModificationFlag)
    {
        telemetry.SetVolatile(SimpleObject(1));
        ASSERT_THAT(telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestWriteModifiedSavesNothingWhenThereAreNoChanges)
    {
        std::array<std::uint8_t, Telemetry::TotalSerializedSize> buffer;
        BitWriter writer(buffer);
        telemetry.SetVolatile(SimpleObject(1));
        telemetry.WriteModified(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture().empty(), Eq(true));
    }

    TEST_F(TelemetryTest, TestWriteModifiedSavesChanges)
    {
        std::array<std::uint8_t, Telemetry::TotalSerializedSize> buffer;
        std::uint8_t expected[] = {0x1, 0x0, 0x0, 0x0};
        BitWriter writer(buffer);
        telemetry.Set(SimpleObject(1));
        telemetry.WriteModified(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        auto span = writer.Capture();
        ASSERT_THAT(span, Eq(gsl::make_span(expected)));
    }

    TEST_F(TelemetryTest, TestMinorChangesAreNotIgnoredUp)
    {
        telemetry.Set(ComplexObject(9, 4));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestMinorChangesAreNotIgnoredDown)
    {
        telemetry.Set(ComplexObject(20, 30));
        telemetry.CommitCapture();
        telemetry.Set(ComplexObject(11, 26));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
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
        std::uint8_t expected[] = {0xee, 0x77, 0xaa, 0x55, 0x0f, 0x00, 0x1a};
        BitWriter writer(buffer);
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
        ASSERT_THAT(complex.ShortValue(), Eq(9u));
        ASSERT_THAT(complex.Byte(), Eq(4u));
        ASSERT_THAT(telemetry.IsModified(), Eq(true));
    }
}
