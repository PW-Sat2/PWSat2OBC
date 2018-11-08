import math

from emulator.beacon_parser.units import TelemetryUnit, unit, TimeFromSeconds
from parser import CategoryParser


@unit('V')
class MPPTVoltage(TelemetryUnit):
    def __init__(self, raw):
        if raw > 4095 or raw < 0:
            raw = 4095
        calculated_v_adc = (raw / 4096.0) * 3.3
        converted = round(calculated_v_adc*((4.7 + 1.0)/(1.0)), 2)

        super(MPPTVoltage, self).__init__(raw, converted)


@unit('A')
class MPPTCurrent(TelemetryUnit):
    def __init__(self, raw):
        if raw > 4095 or raw < 0:
            raw = 4095
        calculated_v_adc = (raw / 4096.0) * 3.3
        converted = round(calculated_v_adc * (1.0 / (0.068 * 50.0)), 3)

        super(MPPTCurrent, self).__init__(raw, converted)


@unit('C')
class MPPTTemperature(TelemetryUnit):
    def __init__(self, raw):
        if raw > 4095 or raw < 0:
            raw = 4095
        calculated_mv_adc = ((raw / 4096.0) * 3.3) * 1000.0
        converted = round(((13.582 - math.sqrt((-13.582 * (-13.582)) + 4 * 0.00433 * (2230.8 - calculated_mv_adc))) / (
            2 * (-0.00433))) + 30.0, 1)

        super(MPPTTemperature, self).__init__(raw, converted)


@unit('V')
class DistributionVoltage(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_v_adc = (raw / 1024.0) * 3.0
        converted = round(calculated_v_adc * ((4.7 + 2.2) / 2.2), 2)

        super(DistributionVoltage, self).__init__(raw, converted)


@unit('A')
class DistributionCurrent(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_v_adc = (raw / 1024.0) * 3.0
        converted = round(calculated_v_adc * (1.0 / (0.025 * 50.0)), 3)

        super(DistributionCurrent, self).__init__(raw, converted)


@unit('C')
class LMT87Temperature(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_mv_adc = ((raw / 1024.0) * 3.0) * 1000.0
        converted = round(((
                               13.582 - math.sqrt(
                                   (-13.582 * (-13.582)) + 4 * 0.00433 * (2230.8 - calculated_mv_adc))) / (
                               2 * (-0.00433))) + 30.0, 1)

        super(LMT87Temperature, self).__init__(raw, converted)


@unit('C')
class TMP121Temperature(TelemetryUnit):
    def __init__(self, raw):
        converted = raw * 0.0625
        super(TMP121Temperature, self).__init__(raw, converted)


@unit('V')
class Local3V3dVoltage(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_v_adc = (raw / 1024.0) * 3.0
        converted = round(calculated_v_adc * 2.0, 2)

        super(Local3V3dVoltage, self).__init__(raw, converted)


@unit('V')
class BATCVoltage(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_v_adc = (raw / 1024.0) * 3.0
        converted = round(calculated_v_adc * ((470.0 + 150.0) / 150.0), 2)

        super(BATCVoltage, self).__init__(raw, converted)


class EPSControllerATelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '14: Controller A', reader, store)

    def get_bit_count(self):
        return 400

    def parse(self):
        self.append('MPPT_X.SOL_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_X.SOL_CURR', 12, value_type=MPPTCurrent)
        self.append('MPPT_X.SOL_OUT_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_X.Temperature', 12, value_type=MPPTTemperature)
        self.append('MPPT_X.State', 3)
        self.append('MPPT_Y+.SOL_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_Y+.SOL_CURR', 12, value_type=MPPTCurrent)
        self.append('MPPT_Y+.SOL_OUT_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_Y+.Temperature', 12, value_type=MPPTTemperature)
        self.append('MPPT_Y+.State', 3)
        self.append('MPPT_Y-.SOL_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_Y-.SOL_CURR', 12, value_type=MPPTCurrent)
        self.append('MPPT_Y-.SOL_OUT_VOLT', 12, value_type=MPPTVoltage)
        self.append('MPPT_Y-.Temperature', 12, value_type=MPPTTemperature)
        self.append('MPPT_Y-.State', 3)
        self.append('DISTR.VOLT_3V3', 10, value_type=DistributionVoltage)
        self.append('DISTR.CURR_3V3', 10, value_type=DistributionCurrent)
        self.append('DISTR.VOLT_5V', 10, value_type=DistributionVoltage)
        self.append('DISTR.CURR_5V', 10, value_type=DistributionCurrent)
        self.append('DISTR.VOLT_VBAT', 10, value_type=DistributionVoltage)
        self.append('DISTR.CURR_VBAT', 10, value_type=DistributionCurrent)
        self.append('DISTR.LCL_STATE', 7)
        self.append('DISTR.LCL_FLAGS', 6)
        self.append('BATC.VOLT_A', 10, value_type=BATCVoltage)
        self.append('BATC.CHRG_CURR', 10, value_type=DistributionCurrent)
        self.append('BATC.DCHRG_CURR', 10, value_type=DistributionCurrent)
        self.append('BATC.Temperature', 10, value_type=LMT87Temperature)
        self.append('BATC.State', 3)
        self.append('BP.Temperature A', 13, two_complement=True, value_type=TMP121Temperature)
        self.append('BP.Temperature B', 13, two_complement=True, value_type=TMP121Temperature)
        self.append('Safety Counter', 8)
        self.append('Power Cycle Count', 16)
        self.append('Uptime', 32, value_type=TimeFromSeconds)
        self.append('Temperature', 10, value_type=LMT87Temperature)
        self.append('SUPP_TEMP', 10, value_type=LMT87Temperature)
        self.append('ControllerB.3V3d', 10, value_type=Local3V3dVoltage)
        self.append('DCDC3V3.Temperature', 10, value_type=LMT87Temperature)
        self.append('DCDC5V.Temperature', 10, value_type=LMT87Temperature)
