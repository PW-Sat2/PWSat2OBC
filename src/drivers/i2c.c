#include <stddef.h>

#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>

#include "drivers/i2c.h"

#include "io_map.h"
#include "drivers/swo.h"

I2C_TransferReturn_TypeDef i2cTransfer(I2C_TransferSeq_TypeDef * seq)
{
	I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C, seq);

	switch (ret)
	{
		case i2cTransferInProgress:
			swoPuts("Trasfer in progress");
			break;
		case i2cTransferDone:
			swoPuts("Transfer done\n\r");
			break;
		default:
			swoPrintf("Transfer error: %d\n\r", ret);
			break;
	}

	do
	{
		swoPuts(".");
		ret = I2C_Transfer(I2C);
	}
	while (ret == i2cTransferInProgress);

	swoPuts("\n");

	switch (ret)
	{
		case i2cTransferDone:
			swoPuts("Transfer done\n\r");
			break;
		default:
			swoPrintf("Transfer error: %d\n\r", ret);
			break;
	}

	return ret;
}

void i2cInit(void)
{
	CMU_ClockEnable(cmuClock_I2C1, true);

	GPIO_PinModeSet(I2C_PORT, I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
	GPIO_PinModeSet(I2C_PORT, I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

	I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
	init.clhr = i2cClockHLRStandard;
	init.enable = true;

	I2C_Init(I2C, &init);
	I2C->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | I2C_LOCATION;
}

I2C_TransferReturn_TypeDef i2cWrite(uint8_t address, uint8_t * inData, uint8_t length)
{
	I2C_TransferSeq_TypeDef seq =
			{
					.addr = address,
					.flags = I2C_FLAG_WRITE,
					.buf =
							{
									{ .len = length, .data = inData },
									{ .len = 0, .data = NULL }
							}
			};

	return i2cTransfer(&seq);
}
