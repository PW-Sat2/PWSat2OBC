from emulator import Emulator
import IPython.lib.guisupport
from emulator.antenna import AntennaModule, AntennasModule
from emulator.eps import EPSModule
from emulator.comm_rtc import CommRTCModule
from emulator.payload import PayloadModule


def _setup_emulator(system):
    emulator_modules = [
        AntennasModule(system),
        CommRTCModule(system),
        EPSModule(system),
        PayloadModule(system)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

_setup_emulator(system)