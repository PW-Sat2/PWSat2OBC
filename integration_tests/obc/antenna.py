import re
from enum import Enum, unique
from .obc_mixin import OBCMixin, command, decode_return


@unique
class AntennaId(Enum):
    Auto = 0
    Antenna1 = 1
    Antenna2 = 2
    Antenna3 = 3
    Antenna4 = 4

    def __str__(self):
        map = {
            self.Auto: "auto",
            self.Antenna1: "1",
            self.Antenna2: "2",
            self.Antenna3: "3",
            self.Antenna4: "4",
        }

        return map[self]


@unique
class AntennaChannel(Enum):
    Primary = 0
    Backup = 1

    def __str__(self):
        map = {
            self.Primary: "primary",
            self.Backup: "backup",
        }

        return map[self]


@unique
class OverrideSwitches(Enum):
    Disabled = 0
    Enabled = 1

    def __str__(self):
        map = {
            self.Disabled: "",
            self.Enabled: "override",
        }

        return map[self]


class AntennaStatus:
    def __init__(self, status):
        self.Status = status
        self.DeploymentState = [False, False, False, False]
        self.DeploymentInProgress = [False, False, False, False]
        self.DeploymentTimeReached = [False, False, False, False]
        self.SystemArmed = False
        self.IgnoringSwitches = False
        self.IndependentBurn = False


class AntennaTelemetry(object):
    def __init__(self):
        self.ActivationCount = [0, 0, 0, 0, 0, 0, 0, 0]
        self.ActivationTime = [0, 0, 0, 0, 0, 0, 0, 0]


class AntennaMixin(OBCMixin):
    def __init__(self):
        pass

    @command("antenna_deploy {0} {1} {2}")
    def antenna_deploy(self, channel, antennaId, override):
        pass

    @command("antenna_cancel {0}")
    def antenna_cancel_deployment(self, channel):
        pass

    def _parse_deployment_state(result):
        lines = result.split("\n")
        lines = map(lambda x: x.split(":"), lines)
        lines = map(lambda (k, v): (k, v.strip().split(' ')), lines)
        lines = map(lambda (k,v): (k, map(int, v)), lines)
        parts = dict(lines)

        operation_status = parts['Status'][0]
        if operation_status != 0:
            return AntennaStatus(False)

        parsed_result = AntennaStatus(True)
        parsed_result.DeploymentState = parts['Deployment status']
        parsed_result.DeploymentInProgress = parts['Deployment active']
        parsed_result.DeploymentTimeReached = parts['Deployment time reached']
        parsed_result.IgnoringSwitches = parts['Ignoring switches'][0]
        parsed_result.IndependentBurn = parts['Independent burn'][0]
        parsed_result.SystemArmed = parts['Armed'][0]
        return parsed_result

    @decode_return(_parse_deployment_state)
    @command("antenna_get_status {0}")
    def antenna_get_status(self, channel):
        # type: (int) -> AntennaStatus
        pass

    @staticmethod
    def extract_value(string):
        m = re.search('([^:]+):\ *\'(\w+)\'', string)
        return int(m.group(2))

    def _parse_telemetry(result):
        telemetry = AntennaTelemetry()
        parts = result.split("\n")
        telemetry.ActivationCount[0] = AntennaMixin.extract_value(parts[0])
        telemetry.ActivationCount[1] = AntennaMixin.extract_value(parts[1])
        telemetry.ActivationCount[2] = AntennaMixin.extract_value(parts[2])
        telemetry.ActivationCount[3] = AntennaMixin.extract_value(parts[3])
        telemetry.ActivationCount[4] = AntennaMixin.extract_value(parts[4])
        telemetry.ActivationCount[5] = AntennaMixin.extract_value(parts[5])
        telemetry.ActivationCount[6] = AntennaMixin.extract_value(parts[6])
        telemetry.ActivationCount[7] = AntennaMixin.extract_value(parts[7])
        telemetry.ActivationTime[0] = AntennaMixin.extract_value(parts[8])
        telemetry.ActivationTime[1] = AntennaMixin.extract_value(parts[9])
        telemetry.ActivationTime[2] = AntennaMixin.extract_value(parts[10])
        telemetry.ActivationTime[3] = AntennaMixin.extract_value(parts[11])
        telemetry.ActivationTime[4] = AntennaMixin.extract_value(parts[12])
        telemetry.ActivationTime[5] = AntennaMixin.extract_value(parts[13])
        telemetry.ActivationTime[6] = AntennaMixin.extract_value(parts[14])
        telemetry.ActivationTime[7] = AntennaMixin.extract_value(parts[15])
        return telemetry

    @decode_return(_parse_telemetry)
    @command("antenna_get_telemetry")
    def antenna_get_telemetry(self):
        pass
