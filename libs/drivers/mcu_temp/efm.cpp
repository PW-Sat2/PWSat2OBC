#include "efm.hpp"
#include <em_adc.h>
#include <em_cmu.h>
#include "system.h"

namespace temp
{
    std::uint16_t ADCTemperatureReader::ReadRawInternal()
    {
        CMU_ClockEnable(cmuClock_ADC0, true);

        ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
        ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;

        init.timebase = ADC_TimebaseCalc(0);
        init.prescale = ADC_PrescaleCalc(400000, 0);

        initSingle.input = adcSingleInputTemp;
        initSingle.reference = adcRef1V25;
        initSingle.resolution = adcRes12Bit;

        ADC_Init(ADC0, &init);
        ADC_InitSingle(ADC0, &initSingle);

        ADC_Start(ADC0, adcStartSingle);

        while (!has_flag(ADC0->STATUS, ADC_STATUS_SINGLEDV))
            ;

        auto sample = ADC0->SINGLEDATA;

        CMU_ClockEnable(cmuClock_ADC0, false);

        return sample;
    }

    BitValue<std::uint16_t, 12> ADCTemperatureReader::ReadRaw()
    {
        return BitValue<std::uint16_t, 12>(ReadRawInternal());
    }

    std::int16_t ADCTemperatureReader::ReadCelsius()
    {
        auto adcSample = this->ReadRawInternal();

        /* Factory calibration temperature from device information page. */
        std::int16_t cal_temp_0 = ((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);

        std::int16_t cal_value_0 = ((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK) >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);

        /* Temperature gradient (from datasheet) */
        constexpr std::int8_t t_grad = -6.3 * 10;

        return (cal_temp_0 * t_grad - 10 * (cal_value_0 - adcSample)) / t_grad;
    }
}
