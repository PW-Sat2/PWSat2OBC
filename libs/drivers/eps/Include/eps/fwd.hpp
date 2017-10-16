#ifndef LIBS_DRIVERS_EPS_INCLUDE_EPS_FWD_HPP_
#define LIBS_DRIVERS_EPS_INCLUDE_EPS_FWD_HPP_

namespace devices
{
    namespace eps
    {
        struct IEPSDriver;
        class EPSDriver;

        /**
         * @brief Available LCLs
         * @ingroup eps
         */
        enum class LCL : std::uint8_t
        {
            TKMain = 0x01,      //!< TKMain
            SunS = 0x02,        //!< SunS
            CamNadir = 0x03,    //!< CamNadir
            CamWing = 0x04,     //!< CamWing
            SENS = 0x05,        //!< SENS
            AntennaMain = 0x06, //!< AntennaMain
            IMTQ = 0x07,        //!< IMTQ
            TKRed = 0x11,       //!< TKRed
            AntennaRed = 0x12   //!< AntennaRed
        };

        /**
         * @brief Available BurnSwitches
         * @ingroup eps
         */
        enum class BurnSwitch : std::uint8_t
        {
            Sail = 0x1, //!< Sail
            SADS = 0x2  //!< SADS
        };

        /**
         * @brief Error codes
         * @ingroup eps
         */
        enum class ErrorCode : std::uint16_t
        {
            NoError = 0,                    //!< NoError
            CommunicationFailure = 0xFF + 1 //!< CommunicationFailure
        };
    }
}

#endif /* LIBS_DRIVERS_EPS_INCLUDE_EPS_FWD_HPP_ */
