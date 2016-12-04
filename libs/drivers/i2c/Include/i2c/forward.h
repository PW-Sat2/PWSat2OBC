#ifndef LIBS_DRIVERS_I2C_INCLUDE_I2C_FORWARD_H_
#define LIBS_DRIVERS_I2C_INCLUDE_I2C_FORWARD_H_

namespace drivers
{
    namespace i2c
    {
        enum class I2CResult;
        struct II2CBus;
        class I2CLowLevelBus;
        struct I2CInterface;
        class I2CFallbackBus;
        class I2CErrorHandlingBus;
    }
}

#endif /* LIBS_DRIVERS_I2C_INCLUDE_I2C_FORWARD_H_ */
