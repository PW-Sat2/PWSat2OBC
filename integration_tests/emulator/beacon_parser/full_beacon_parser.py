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

class FullBeaconParser:
    def GetParsers(self, file_tree):
        return [StartupParser(file_tree),
                ProgramStateParser(file_tree),
                TimeState(file_tree),
                ErrorCountingTelemetry(file_tree),
                ScrubbingTelemetryParser(file_tree),
                FileSystemTelemetryParser(file_tree),
                AntennaTelemetryParser(file_tree),
                ExperimentTelemetryParser(file_tree),
                GyroscopeTelemetryParser(file_tree),
                CommTelemetryParser(file_tree),
                GpioStateParser(file_tree),
                McuTemperatureParser(file_tree),
                EPSControllerATelemetryParser(file_tree),
                EPSControllerBTelemetryParser(file_tree),
                ImtqMagnetometersTelemetryParser(file_tree),
                ImtqCoilActiveTelemetryParser(file_tree),
                ImtqDipoleTelemetryParser(file_tree),
                ImtqBDotTelemetryParser(file_tree),
                ImtqHousekeepingTelemetryParser(file_tree),
                ImtqCoilsTelemetryParser(file_tree),
                ImtqTemperatureTelemetryParser(file_tree),
                ImtqStateTelemetryParser(file_tree),
                ImtqSelfTestTelemetryParser(file_tree)]
