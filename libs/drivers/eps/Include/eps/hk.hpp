#ifndef LIBS_DRIVERS_EPS_INCLUDE_EPS_HK_HPP_
#define LIBS_DRIVERS_EPS_INCLUDE_EPS_HK_HPP_

#include "base/reader.h"
#include "utils.h"

namespace devices
{
    namespace eps
    {
        namespace hk
        {
            enum class MPPT_STATE : std::uint8_t
            {
                A = 1,
                B = 2,
                C = 4,
                D = 8,
                E = 16,
                F = 32
            };

            enum class DISTR_LCL_STATE : std::uint8_t
            {
                A = 1,
                B = 2,
                C = 4,
                D = 8,
                E = 16,
                F = 32,
                G = 64,
                H = 128
            };

            enum class DISTR_LCL_FLAGB : std::uint8_t
            {
                A = 1,
                B = 2,
                C = 4,
                D = 8,
                E = 16,
                F = 32,
                G = 64,
                H = 128
            };

            enum class BATC_STATE : std::uint8_t
            {
                A = 1,
                B = 2,
                C = 4,
                D = 8,
                E = 16,
                F = 32,
                G = 64,
                H = 128
            };

            struct OtherController
            {
                void ReadFrom(Reader& reader);

                uint10_t VOLT_3V3d;
            };

            struct ThisController
            {
                void ReadFrom(Reader& reader);

                std::uint8_t ERR;
                std::uint16_t PWR_CYCLES;
                std::uint32_t UPTIME;
                uint10_t TEMP;
            };

            struct HouseheepingControllerA
            {
                bool ReadFrom(Reader& reader);

                struct MPPT_HK
                {
                    void ReadFrom(Reader& reader);

                    uint12_t SOL_VOLT;
                    uint12_t SOL_CURR;
                    uint12_t SOL_OUT_VOLT;
                    uint12_t TEMP;
                    MPPT_STATE STATE;
                };

                MPPT_HK MPPT_X;

                MPPT_HK MPPT_Y_PLUS;

                MPPT_HK MPPT_Y_MINUS;

                struct DISTR_HK
                {
                    void ReadFrom(Reader& reader);

                    uint10_t TEMP;
                    uint10_t VOLT_3V3;
                    uint10_t CURR_3V3;
                    uint10_t VOLT_5V;
                    uint10_t CURR_5V;
                    uint10_t VOLT_VBAT;
                    uint10_t CURR_VBAT;
                    DISTR_LCL_STATE LCL_STATE;
                    DISTR_LCL_FLAGB LCL_FLAGB;
                };

                DISTR_HK DISTR;

                struct BATC_HK
                {
                    void ReadFrom(Reader& reader);

                    uint10_t VOLT_A;
                    uint10_t CHRG_CURR;
                    uint10_t DCHRG_CURR;
                    uint10_t TEMP;
                    BATC_STATE STATE;
                };

                BATC_HK BATC;

                struct BP_HK
                {
                    void ReadFrom(Reader& reader);

                    uint12_t TEMP_A;
                    uint12_t TEMP_B;
                };

                BP_HK BP;

                OtherController CTRLB;

                ThisController CTRLA;

                struct DCDC_HK
                {
                    void ReadFrom(Reader& reader);

                    uint10_t TEMP;
                };

                DCDC_HK DCDC3V3;
                DCDC_HK DCDC5V;
            };

            struct HouseheepingControllerB
            {
                bool ReadFrom(Reader& reader);

                struct BP_HK
                {
                    void ReadFrom(Reader& reader);

                    uint10_t TEMP_C;
                };

                BP_HK BP;

                struct BATC_HK
                {
                    void ReadFrom(Reader& reader);

                    uint10_t VOLT_B;
                };

                BATC_HK BATC;

                OtherController CTRLA;
                ThisController CTRLB;
            };
        }
    }
}

#endif /* LIBS_DRIVERS_EPS_INCLUDE_EPS_HK_HPP_ */
