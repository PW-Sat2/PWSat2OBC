from emulator.beacon_parser.units import TimeFromMilliseconds

import eps_controller_a_telemetry_parser as EPSControllerATelemetryParser
import eps_controller_b_telemetry_parser as EPSControllerBTelemetryParser

from parser import CategoryParser
from deep_sleep_beacon_parser import DeepSleepBeaconFrameParser

class LittleOryxExtendedBeaconFrameParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '02: Little Oryx Extended Beacon', reader, store)

    def get_bit_count(self):
        return 80

    def parse(self):
        # Time, BATC.VOLT_A, BATC.VOLT_B, Reboot to normal fields parsed by original Deep Sleep beacon parser

        self.append('DISTR.CURR_3V3', 16, value_type=EPSControllerATelemetryParser.DistributionCurrent)
        self.append('DISTR.CURR_5V', 16, value_type=EPSControllerATelemetryParser.DistributionCurrent)
        self.append('DISTR.CURR_VBAT', 16, value_type=EPSControllerATelemetryParser.DistributionCurrent)

        self.append('BP.Temperature A', 16, two_complement=True, value_type=EPSControllerATelemetryParser.TMP121Temperature)
        self.append('BP.Temperature B', 16, two_complement=True, value_type=EPSControllerATelemetryParser.TMP121Temperature)
       
class LittleOryxBeaconParser:
    def GetParsers(self, reader, store):
        return [DeepSleepBeaconFrameParser(reader, store), 
            LittleOryxExtendedBeaconFrameParser(reader, store)]
