from emulator.beacon_parser.units import TimeFromMilliseconds

import eps_controller_a_telemetry_parser as EPSControllerATelemetryParser
import eps_controller_b_telemetry_parser as EPSControllerBTelemetryParser

from parser import CategoryParser

class DeepSleepBeaconFrameParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '0: Deep Sleep Beacon', reader, store)

    def get_bit_count(self):
        return 96

    def parse(self):
        self.append('Time', 32, value_type=TimeFromMilliseconds)
        self.append('BATC.VOLT_A', 16, value_type=EPSControllerATelemetryParser.BATCVoltage)
        self.append('BATC.VOLT_B', 16, value_type=EPSControllerBTelemetryParser.BATCVoltage)
        self.append('Reboot to normal', 32)
       
class DeepSleepBeaconParser:
    def GetParsers(self, reader, store):
        return [DeepSleepBeaconFrameParser(reader, store)]
