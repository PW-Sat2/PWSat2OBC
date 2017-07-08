from error_counting_telementry import  ErrorCountingTelementry
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


class FullBeaconParser:
    def GetParsers(self, file_tree):
        return [StartupParser(file_tree),
                ProgramStateParser(file_tree),
                TimeState(file_tree),
                ErrorCountingTelementry(file_tree),
                FileSystemTelemetryParser(file_tree),
                AntennaTelemetryParser(file_tree),
                ExperimentTelemetryParser(file_tree),
                GyroscopeTelemetryParser(file_tree),
                CommTelemetryParser(file_tree),
                GpioStateParser(file_tree),
                McuTemperatureParser(file_tree),
                EPSControllerATelemetryParser(file_tree),
                EPSControllerBTelemetryParser(file_tree)]

