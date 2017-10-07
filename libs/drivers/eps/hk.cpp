#include "hk.hpp"
#include "base/BitWriter.hpp"
#include "base/reader.h"

namespace devices
{
    namespace eps
    {
        namespace hk
        {
            void OtherControllerState::ReadFrom(Reader& reader)
            {
                this->VOLT_3V3d = reader.ReadWordLE();
            }

            void OtherControllerState::Write(BitWriter& writer) const
            {
                writer.Write(this->VOLT_3V3d);
            }

            ThisControllerState::ThisControllerState() : powerCycleCount(0), uptime(0), safetyCounter(0)
            {
            }

            void ThisControllerState::ReadFrom(Reader& reader)
            {
                this->safetyCounter = reader.ReadByte();
                this->powerCycleCount = reader.ReadWordLE();
                this->uptime = reader.ReadDoubleWordLE();
                this->temperature = reader.ReadWordLE();
                this->suppTemp = reader.ReadWordLE();
            }

            void ThisControllerState::Write(BitWriter& writer) const
            {
                writer.Write(this->safetyCounter);
                writer.Write(this->powerCycleCount);
                writer.Write(this->uptime);
                writer.Write(this->temperature);
                writer.Write(this->suppTemp);
            }

            void DCDC_HK::ReadFrom(Reader& reader)
            {
                this->temperature = reader.ReadWordLE();
            }

            void DCDC_HK::Write(BitWriter& writer) const
            {
                writer.Write(this->temperature);
            }

            DISTR_HK::DISTR_HK() : LCL_STATE(DISTR_LCL_STATE::None), LCL_FLAGB(DISTR_LCL_FLAGB::None)
            {
            }

            void DISTR_HK::ReadFrom(Reader& reader)
            {
                this->CURR_3V3 = reader.ReadWordLE();
                this->VOLT_3V3 = reader.ReadWordLE();
                this->CURR_5V = reader.ReadWordLE();
                this->VOLT_5V = reader.ReadWordLE();
                this->CURR_VBAT = reader.ReadWordLE();
                this->VOLT_VBAT = reader.ReadWordLE();
                this->LCL_STATE = static_cast<decltype(this->LCL_STATE)>(reader.ReadByte());
                this->LCL_FLAGB = static_cast<decltype(this->LCL_FLAGB)>(reader.ReadByte());
            }

            void DISTR_HK::Write(BitWriter& writer) const
            {
                writer.Write(this->VOLT_3V3);
                writer.Write(this->CURR_3V3);
                writer.Write(this->VOLT_5V);
                writer.Write(this->CURR_5V);
                writer.Write(this->VOLT_VBAT);
                writer.Write(this->CURR_VBAT);
                writer.WriteWord(num(this->LCL_STATE), 6);
                writer.WriteWord(num(this->LCL_FLAGB), 6);
            }

            MPPT_HK::MPPT_HK() : MpptState(MPPT_STATE::None)
            {
            }

            void MPPT_HK::ReadFrom(Reader& reader)
            {
                this->SOL_CURR = reader.ReadWordLE();
                this->SOL_VOLT = reader.ReadWordLE();
                this->SOL_OUT_VOLT = reader.ReadWordLE();
                this->Temperature = reader.ReadWordLE();
                this->MpptState = static_cast<decltype(this->MpptState)>(reader.ReadByte());
            }

            void MPPT_HK::Write(BitWriter& writer) const
            {
                writer.Write(this->SOL_VOLT);
                writer.Write(this->SOL_CURR);
                writer.Write(this->SOL_OUT_VOLT);
                writer.Write(this->Temperature);
                writer.WriteWord(num(this->MpptState), StateBitSize);
            }

            BATCPrimaryState::BATCPrimaryState() : State(BATC_STATE::None)
            {
            }

            void BATCPrimaryState::ReadFrom(Reader& reader)
            {
                this->VOLT_A = reader.ReadWordLE();
                this->ChargeCurrent = reader.ReadWordLE();
                this->DischargeCurrent = reader.ReadWordLE();
                this->Temperature = reader.ReadWordLE();
                this->State = static_cast<decltype(this->State)>(reader.ReadByte());
            }

            void BATCPrimaryState::Write(BitWriter& writer) const
            {
                writer.Write(this->VOLT_A);
                writer.Write(this->ChargeCurrent);
                writer.Write(this->DischargeCurrent);
                writer.Write(this->Temperature);
                writer.WriteWord(num(this->State), 3);
            }

            void BatteryPackPrimaryState::ReadFrom(Reader& reader)
            {
                this->temperatureA = reader.ReadWordLE();
                this->temperatureB = reader.ReadWordLE();
            }

            void BatteryPackPrimaryState::Write(BitWriter& writer) const
            {
                writer.Write(this->temperatureA);
                writer.Write(this->temperatureB);
            }

            void BatteryPackSecondaryState::ReadFrom(Reader& reader)
            {
                this->temperatureC = reader.ReadWordLE();
            }

            void BatteryPackSecondaryState::Write(BitWriter& writer) const
            {
                writer.Write(this->temperatureC);
            }
            void BATCSecondaryState::ReadFrom(Reader& reader)
            {
                this->voltB = reader.ReadWordLE();
            }

            void BATCSecondaryState::Write(BitWriter& writer) const
            {
                writer.Write(this->voltB);
            }

            bool ControllerATelemetry::ReadFrom(Reader& reader)
            {
                this->WhoAmI = reader.ReadByte();
                this->mpptX.ReadFrom(reader);
                this->mpptYPlus.ReadFrom(reader);
                this->mpptYMinus.ReadFrom(reader);
                this->distr.ReadFrom(reader);
                this->batc.ReadFrom(reader);
                this->bp.ReadFrom(reader);
                this->other.ReadFrom(reader);
                this->current.ReadFrom(reader);
                this->dcdc3V3.ReadFrom(reader);
                this->dcdc5V.ReadFrom(reader);
                return reader.Status();
            }

            void ControllerATelemetry::Write(BitWriter& writer) const
            {
                this->mpptX.Write(writer);
                this->mpptYPlus.Write(writer);
                this->mpptYMinus.Write(writer);
                this->distr.Write(writer);
                this->batc.Write(writer);
                this->bp.Write(writer);
                this->current.Write(writer);
                this->other.Write(writer);
                this->dcdc3V3.Write(writer);
                this->dcdc5V.Write(writer);
            }

            bool ControllerBTelemetry::ReadFrom(Reader& reader)
            {
                this->WhoAmI = reader.ReadByte();
                this->bp.ReadFrom(reader);
                this->batc.ReadFrom(reader);
                this->other.ReadFrom(reader);
                this->current.ReadFrom(reader);
                return reader.Status();
            }

            void ControllerBTelemetry::Write(BitWriter& writer) const
            {
                this->bp.Write(writer);
                this->batc.Write(writer);
                this->current.Write(writer);
                this->other.Write(writer);
            }
        }
    }
}
