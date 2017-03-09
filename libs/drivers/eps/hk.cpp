#include "hk.hpp"

namespace devices
{
    namespace eps
    {
        namespace hk
        {
            bool HouseheepingControllerA::ReadFrom(Reader& reader)
            {
                this->MPPT_X.ReadFrom(reader);
                this->MPPT_Y_PLUS.ReadFrom(reader);
                this->MPPT_Y_MINUS.ReadFrom(reader);
                this->DISTR.ReadFrom(reader);
                this->BATC.ReadFrom(reader);
                this->BP.ReadFrom(reader);
                this->CTRLB.ReadFrom(reader);
                this->CTRLA.ReadFrom(reader);
                this->DCDC3V3.ReadFrom(reader);
                this->DCDC5V.ReadFrom(reader);

                return reader.Status();
            }

            void HouseheepingControllerA::MPPT_HK::ReadFrom(Reader& reader)
            {
                this->SOL_CURR = reader.ReadWordLE();
                this->SOL_VOLT = reader.ReadWordLE();
                this->SOL_OUT_VOLT = reader.ReadWordLE();
                this->TEMP = reader.ReadWordLE();
                this->STATE = static_cast<decltype(this->STATE)>(reader.ReadByte());
            }

            void HouseheepingControllerA::DISTR_HK::ReadFrom(Reader& reader)
            {
                this->CURR_3V3 = reader.ReadWordLE();
                this->VOLT_3V3 = reader.ReadWordLE();
                this->CURR_5V = reader.ReadWordLE();
                this->VOLT_5V = reader.ReadWordLE();
                this->CURR_VBAT = reader.ReadWordLE();
                this->VOLT_VBAT = reader.ReadWordLE();
                this->TEMP = reader.ReadWordLE();
                this->LCL_STATE = static_cast<decltype(this->LCL_STATE)>(reader.ReadByte());
                this->LCL_FLAGB = static_cast<decltype(this->LCL_FLAGB)>(reader.ReadByte());
            }

            void HouseheepingControllerA::BATC_HK::ReadFrom(Reader& reader)
            {
                this->VOLT_A = reader.ReadWordLE();
                this->CHRG_CURR = reader.ReadWordLE();
                this->DCHRG_CURR = reader.ReadWordLE();
                this->TEMP = reader.ReadWordLE();
                this->STATE = static_cast<decltype(this->STATE)>(reader.ReadByte());
            }

            void HouseheepingControllerA::BP_HK::ReadFrom(Reader& reader)
            {
                this->TEMP_A = reader.ReadWordLE();
                this->TEMP_B = reader.ReadWordLE();
            }

            void OtherController::ReadFrom(Reader& reader)
            {
                this->VOLT_3V3d = reader.ReadWordLE();
            }

            void ThisController::ReadFrom(Reader& reader)
            {
                this->ERR = reader.ReadByte();
                this->PWR_CYCLES = reader.ReadWordLE();
                this->UPTIME = reader.ReadDoubleWordLE();
                this->TEMP = reader.ReadWordLE();
            }

            void HouseheepingControllerA::DCDC_HK::ReadFrom(Reader& reader)
            {
                this->TEMP = reader.ReadWordLE();
            }

            bool HouseheepingControllerB::ReadFrom(Reader& reader)
            {
                this->BP.ReadFrom(reader);
                this->BATC.ReadFrom(reader);
                this->CTRLA.ReadFrom(reader);
                this->CTRLB.ReadFrom(reader);

                return reader.Status();
            }

            void HouseheepingControllerB::BP_HK::ReadFrom(Reader& reader)
            {
                this->TEMP_C = reader.ReadWordLE();
            }

            void HouseheepingControllerB::BATC_HK::ReadFrom(Reader& reader)
            {
                this->VOLT_B = reader.ReadWordLE();
            }
        }
    }
}
