from error_counting_telemetry import  ErrorCountingTelemetry
from program_state import ProgramStateParser
from startup_parser import StartupParser
from time_state import TimeState
from file_system_telemetry_parser import FileSystemTelemetryParser
from antenna_telemetry_parser import AntennaTelemetryParser
from experiment_telemetry_parser import ExperimentTelemetryParser
from gyroscope_telemetry_parser import GyroscopeTelemetryParser
from comm_telemetry_parser import CommTelemetryParser
from gpio_state_parser import GpioStateParser
from mcu_temperature_parser import McuTemperatureParser
from eps_controller_a_telemetry_parser import EPSControllerATelemetryParser
from eps_controller_b_telemetry_parser import EPSControllerBTelemetryParser
from scrubbing_telemetry_parser import ScrubbingTelemetryParser
from imtq_bdot_telemetry_parser import ImtqBDotTelemetryParser
from imtq_coil_active import ImtqCoilActiveTelemetryParser
from imtq_coils_telemetry_parser import ImtqCoilsTelemetryParser
from imtq_dipole_telemetry_parser import ImtqDipoleTelemetryParser
from imtq_housekeeping_telemetry_parser import ImtqHousekeepingTelemetryParser
from imtq_magnetometers_telemetry_parser import ImtqMagnetometersTelemetryParser
from imtq_self_test_telemetry_parser import ImtqSelfTestTelemetryParser
from imtq_state_telemetry_parser import ImtqStateTelemetryParser
from imtq_temperature_telemetry_parser import ImtqTemperatureTelemetryParser
from system_parser import SystemParser


class FullBeaconParser:
    def GetParsers(self, reader, store):
        return [StartupParser(reader, store),
                ProgramStateParser(reader, store),
                TimeState(reader, store),
                ErrorCountingTelemetry(reader, store),
                ScrubbingTelemetryParser(reader, store),
                SystemParser(reader, store),
                FileSystemTelemetryParser(reader, store),
                AntennaTelemetryParser(reader, store),
                ExperimentTelemetryParser(reader, store),
                GyroscopeTelemetryParser(reader, store),
                CommTelemetryParser(reader, store),
                GpioStateParser(reader, store),
                McuTemperatureParser(reader, store),
                EPSControllerATelemetryParser(reader, store),
                EPSControllerBTelemetryParser(reader, store),
                ImtqMagnetometersTelemetryParser(reader, store),
                ImtqCoilActiveTelemetryParser(reader, store),
                ImtqDipoleTelemetryParser(reader, store),
                ImtqBDotTelemetryParser(reader, store),
                ImtqHousekeepingTelemetryParser(reader, store),
                ImtqCoilsTelemetryParser(reader, store),
                ImtqTemperatureTelemetryParser(reader, store),
                ImtqStateTelemetryParser(reader, store),
                ImtqSelfTestTelemetryParser(reader, store)]
