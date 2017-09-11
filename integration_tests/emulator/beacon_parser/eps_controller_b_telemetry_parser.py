from emulator.beacon_parser import resistance_sensors
from emulator.beacon_parser.units import TelemetryUnit, unit, TimeFromSeconds
from parser import CategoryParser
from .eps_controller_a_telemetry_parser import BATCVoltage, LMT87Temperature, Local3V3dVoltage


@unit('C')
class PT1000Temperature(TelemetryUnit):
    def __init__(self, raw):
        if raw > 1023 or raw < 0:
            raw = 1023
        calculated_v_adc = (raw / 1024.0) * 3.0
        pt1000_resistance = calculated_v_adc / ((3.0 - calculated_v_adc) / 3320.68)
        converted = round(resistance_sensors.pt1000_res_to_temp(pt1000_resistance), 1)

        super(PT1000Temperature, self).__init__(raw, converted)


class EPSControllerBTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '15: Controller B', reader, store)

    def get_bit_count(self):
        return 10 + 10 + 8 + 16 + 32 + 10 + 10 + 10

    def parse(self):
        self.append("BP.Temperature", 10, value_type=PT1000Temperature)
        self.append("BATC.VOLT_B", 10, value_type=BATCVoltage)
        self.append("Safety Counter", 8)
        self.append("Power Cycle Count", 16)
        self.append("Uptime", 32, value_type=TimeFromSeconds)
        self.append("Temperature", 10, value_type=LMT87Temperature)
        self.append('SUPP_TEMP', 10, value_type=LMT87Temperature)
        self.append("ControllerA.3V3d", 10, value_type=Local3V3dVoltage)
