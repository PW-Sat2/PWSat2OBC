from emulator import Emulator
import IPython.lib.guisupport
from emulator.antenna import AntennaModule
from emulator.eps import EPSModule
from emulator.beacon_rtc import BeaconRTCModule


def _setup_emulator(system):
    emulator_modules = [
        AntennaModule('Primary', (0, 0), system.primary_antenna),
        AntennaModule('Backup', (0,1), system.backup_antenna),
        BeaconRTCModule(system),
        EPSModule(system)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

_setup_emulator(system)