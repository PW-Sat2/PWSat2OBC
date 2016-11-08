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

def AntennaMixin(OBCMixin):
    def __init__(self):
        pass

    @command("antenna deploy {0} {1} {2}")
    def antenna_deploy(self, channel, antennaId, override):
         pass

    @command("antenna deploy cancel")
    def antenna_cancel_deployment(self):
        pass
