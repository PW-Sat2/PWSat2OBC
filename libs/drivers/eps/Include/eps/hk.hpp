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
            /**
             * @defgroup eps_hk EPS Housekeeping
             * @ingroup eps
             *
             * @{
             */

            /**
             * @brief MPPT state
             */
            enum class MPPT_STATE : std::uint8_t
            {
                A = 1,  //!< A
                B = 2,  //!< B
                C = 4,  //!< C
                D = 8,  //!< D
                E = 16, //!< E
                F = 32  //!< F
            };

            /**
             * @brief DISTR LCL state
             */
            enum class DISTR_LCL_STATE : std::uint8_t
            {
                A = 1,  //!< A
                B = 2,  //!< B
                C = 4,  //!< C
                D = 8,  //!< D
                E = 16, //!< E
                F = 32, //!< F
                G = 64, //!< G
                H = 128 //!< H
            };

            /**
             * @brief DISTR LCL flags
             */
            enum class DISTR_LCL_FLAGB : std::uint8_t
            {
                A = 1,  //!< A
                B = 2,  //!< B
                C = 4,  //!< C
                D = 8,  //!< D
                E = 16, //!< E
                F = 32, //!< F
                G = 64, //!< G
                H = 128 //!< H
            };

            /**
             * @brief Battery controller state
             */
            enum class BATC_STATE : std::uint8_t
            {
                A = 1,  //!< A
                B = 2,  //!< B
                C = 4,  //!< C
                D = 8,  //!< D
                E = 16, //!< E
                F = 32, //!< F
                G = 64, //!< G
                H = 128 //!< H
            };

            /**
             * @brief Housekeeping of the 'other' controller
             */
            struct OtherController
            {
                /**
                 * @brief Reads part of input stream
                 * @param reader Reader over input stream
                 */
                void ReadFrom(Reader& reader);

                /** @brief VOLT_3V3d, volts */
                uint10_t VOLT_3V3d;
            };

            /**
             * @brief Housekeeping of the 'this' controller
             */
            struct ThisController
            {
                /**
                 * @brief Reads part of input stream
                 * @param reader Reader over input stream
                 */
                void ReadFrom(Reader& reader);

                /** @brief Error code */
                std::uint8_t ERR;
                /** @brief Number of power cycles */
                std::uint16_t PWR_CYCLES;
                /** @brief Internal timer, seconds*/
                std::uint32_t UPTIME;
                /** @brief Temperature, Celsius */
                uint10_t TEMP;
            };

            /**
             * @brief Housekeeping of controller A
             */
            struct HouseheepingControllerA
            {
                /**
                 * @brief Reads part of input stream
                 * @param reader Reader over input stream
                 * @return true if read correctly
                 */
                bool ReadFrom(Reader& reader);

                /** @brief MPPT status */
                struct MPPT_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief MPPT: SOL_VOLT, volts*/
                    uint12_t SOL_VOLT;
                    /** @brief MPPT: SOL_CURR, mA*/
                    uint12_t SOL_CURR;
                    /** @brief MPPT: SOL_OUT_VOL, volts*/
                    uint12_t SOL_OUT_VOLT;
                    /** @brief MPPT: temperature, Celsius */
                    uint12_t TEMP;
                    /** @brief MPPT: algorithm state s*/
                    MPPT_STATE STATE;
                };

                /** @brief MPPT_X */
                MPPT_HK MPPT_X;
                /** @brief MPPT_Y+*/
                MPPT_HK MPPT_Y_PLUS;
                /** @brief MPPT_Y-*/
                MPPT_HK MPPT_Y_MINUS;

                /** @brief DISTR status*/
                struct DISTR_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief DISTR: temperature, Celsius */
                    uint10_t TEMP;
                    /** @brief DISTR: VOLT_3V3, volts*/
                    uint10_t VOLT_3V3;
                    /** @brief DIStr: CURR_3V3, mA*/
                    uint10_t CURR_3V3;
                    /** @brief DISTR: VOLT_5V, volts*/
                    uint10_t VOLT_5V;
                    /** @brief DISTR: CURR_5V, mA*/
                    uint10_t CURR_5V;
                    /** @brief DISTR: VOLT_VBAT, volts*/
                    uint10_t VOLT_VBAT;
                    /** @brief DISTR: CURR_VBAT, mA*/
                    uint10_t CURR_VBAT;
                    /** @brief DISTR: LCL states*/
                    DISTR_LCL_STATE LCL_STATE;
                    /** @brief DISTR: LCL FlagBs */
                    DISTR_LCL_FLAGB LCL_FLAGB;
                };

                /** @brief DISTR status*/
                DISTR_HK DISTR;

                /** @brief BATC status*/
                struct BATC_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief BATC: VOLT_A, volts*/
                    uint10_t VOLT_A;
                    /** @brief BATC: CHRG_CURR, mA*/
                    uint10_t CHRG_CURR;
                    /** @brief BATC: DCHRH_CURR, mA*/
                    uint10_t DCHRG_CURR;
                    /** @brief BATC: temperature, Celsius */
                    uint10_t TEMP;
                    /** @brief BATC: internal states*/
                    BATC_STATE STATE;
                };

                /** @brief BATC status*/
                BATC_HK BATC;

                /** @brief Battery Pack status*/
                struct BP_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief Temp A, Celsius*/
                    uint12_t TEMP_A;
                    /** @brief Temp B, Celsius*/
                    uint12_t TEMP_B;
                };

                /** @brief Battery Pack status*/
                BP_HK BP;

                /** @brief Controller B status*/
                OtherController CTRLB;

                /** @brief Controller A status*/
                ThisController CTRLA;

                /** @brief DCDC status*/
                struct DCDC_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief DCDC: Temperature, Celsius*/
                    uint10_t TEMP;
                };

                /** @brief DCDC: 3V3*/
                DCDC_HK DCDC3V3;
                /** @brief DCDC: 5V*/
                DCDC_HK DCDC5V;
            };

            /**
             * @brief Housekeeping of controller A
             */
            struct HouseheepingControllerB
            {
                /**
                 * @brief Reads part of input stream
                 * @param reader Reader over input stream
                 * @return true if read correctly
                 */
                bool ReadFrom(Reader& reader);

                /** @brief Battery Pack status*/
                struct BP_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief Battery Pack: Temp C, Celsius*/
                    uint10_t TEMP_C;
                };

                /** @brief Battery Pack status*/
                BP_HK BP;

                /** @brief Battery Controller status*/
                struct BATC_HK
                {
                    /**
                     * @brief Reads part of input stream
                     * @param reader Reader over input stream
                     */
                    void ReadFrom(Reader& reader);

                    /** @brief BATC: temp B, Celsius*/
                    uint10_t VOLT_B;
                };

                /** @brief Battery Controller status*/
                BATC_HK BATC;

                /** @brief Controller A status*/
                OtherController CTRLA;
                /** @brief Controller B status*/
                ThisController CTRLB;
            };

            /** @} */
        }
    }
}

#endif /* LIBS_DRIVERS_EPS_INCLUDE_EPS_HK_HPP_ */
