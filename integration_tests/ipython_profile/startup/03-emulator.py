from emulator import Emulator
import IPython.lib.guisupport
from emulator.antenna import AntennaModule
from emulator.eps import EPSModule
from emulator.comm_rtc import CommRTCModule


def _setup_emulator(system):
    emulator_modules = [
        AntennaModule('Primary', (0, 0), system.primary_antenna),
        AntennaModule('Backup', (0,1), system.backup_antenna),
        CommRTCModule(system),
        EPSModule(system)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

_setup_emulator(system)