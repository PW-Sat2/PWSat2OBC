import IPython.lib.guisupport

from emulator import Emulator
from emulator.antenna import AntennasModule
from emulator.eps import EPSModule
from emulator.payload import PayloadModule
from emulator.rtc import RTCModule
from emulator.comm import CommModule


def _setup_emulator(system):
    emulator_modules = [
        AntennasModule(system),
        RTCModule(system),
        EPSModule(system),
        PayloadModule(system),
        CommModule(system)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

_setup_emulator(system)