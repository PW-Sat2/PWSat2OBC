from .obc_mixin import OBCMixin, command, decode_return
from enum import Enum, unique

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
    Status = False
    DeploymentState = [False, False, False, False]
    DeploymentInProgress = [False, False, False, False]
    SystemArmed = False
    IgnoringSwitches = False

    def __init__(self, status):
        self.Status = status

class AntennaMixin(OBCMixin):
    def __init__(self):
        pass

    @command("antenna_deploy {0} {1} {2}")
    def antenna_deploy(self, channel, antennaId, override):
         pass

    @command("antenna_cancel {0}")
    def antenna_cancel_deployment(self, channel):
        pass

    def parse_deployment_state(result):
        parts = result.split(" ");

        operationStatus = int(parts[0])
        if(operationStatus != 0):
            return AntennaStatus(False)

        parsedResult = AntennaStatus(True)
        for cx in range(4):
            parsedResult.DeploymentState[cx] = int(parts[cx + 1])

        for cx in range(4):
            parsedResult.DeploymentInProgress[cx] = int(parts[cx + 5])

        parsedResult.IgnoringSwitches = int(parts[9])
        parsedResult.SystemArmed = int(parts[10])
        return parsedResult

    @decode_return(parse_deployment_state)
    @command("antenna_get_status {0}")
    def antenna_get_status(self, channel):
        pass
