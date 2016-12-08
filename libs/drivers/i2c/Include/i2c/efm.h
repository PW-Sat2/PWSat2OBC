#ifndef LIBS_DRIVERS_I2C_INCLUDE_I2C_EFM_H_
#define LIBS_DRIVERS_I2C_INCLUDE_I2C_EFM_H_

#include "i2c.h"

namespace drivers
{
    namespace i2c
    {
        /**
         * @ingroup i2c
         *
         * @{
         */

        /** @brief Low-level I2C bus driver */
        class I2CLowLevelBus : public II2CBus
        {
          public:
            /**
             * @brief Setups I2C low-level driver object. No RTOS or hardware initialization is done in constructor
             * @param[in] hw I2C hardware registers set
             * @param[in] location Pins location to use
             * @param[in] port GPIO port to use
             * @param[in] sdaPin Number of GPIO pin to use for SDA line
             * @param[in] sclPin Number of GPIO pin to use for SCL line
             * @param[in] clock Clock used by selected hardware interface
             * @param[in] irq IRQ number used by selected hardware interface
             */
            I2CLowLevelBus(I2C_TypeDef* hw,
                uint16_t location,
                GPIO_Port_TypeDef port,
                uint16_t sdaPin,
                uint16_t sclPin,
                CMU_Clock_TypeDef clock,
                IRQn_Type irq);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

            /**
             * @brief Initializes RTOS and hardware components of I2C low-level driver
             */
            void Initialize();

            /**
             * @brief Interrupt handler for I2C hardware
             */
            void IRQHandler();

          private:
            /**
             * @brief Executes single I2C transfer
             * @param[in] bus I2C bus
             * @param[in] seq Transfer sequence definition
             * @return Transfer result
             */
            I2CResult ExecuteTransfer(I2C_TransferSeq_TypeDef* seq);

            /**
             * @brief Checks if SCL line is latched at low level
             * @param[in] bus I2C bus
             * @return true if SCL line is latched
             */
            bool IsSclLatched();

            /** @brief Pointer to hardware registers */
            void* HWInterface;

            /**
             * @brief GPIO used by this I2C interface
             */
            struct
            {
                /** @brief Peripheral clock */
                CMU_Clock_TypeDef Clock;
                /** @brief Peripheral interrupt number */
                IRQn_Type IRQn;
                /** @brief Peripheral location number */
                uint16_t Location;
                /** @brief Used port */
                GPIO_Port_TypeDef Port;
                /** @brief Number of pin connected to SCL line */
                uint16_t SCL;
                /** @brief Number of pin connected to SDA line */
                uint16_t SDA;
            } _io;

            /** @brief Lock used to synchronize access to bus */
            OSSemaphoreHandle _lock;

            /** @brief Single-element queue storing results of transfers */
            Queue<I2C_TransferReturn_TypeDef, 1> _resultQueue;
        };

        /** @} */
    }
}
#endif /* LIBS_DRIVERS_I2C_INCLUDE_I2C_EFM_H_ */
