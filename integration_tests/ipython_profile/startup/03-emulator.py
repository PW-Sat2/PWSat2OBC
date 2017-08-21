import IPython.lib.guisupport

from emulator import Emulator
from emulator.antenna import AntennasModule
from emulator.beacon import BeaconModule
from emulator.eps import EPSModule
from emulator.last_frames import LastFramesModule
from emulator.payload import PayloadModule
from emulator.rtc import RTCModule
from emulator.comm import CommModule

last_frames = []

last_beacon = {}

def _setup_emulator(system):
    emulator_modules = [
        BeaconModule(last_beacon),
        AntennasModule(system),
        RTCModule(system),
        EPSModule(system),
        PayloadModule(system),
        CommModule(system),
        LastFramesModule(last_frames)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

    def store_last_frame(comm, frame):
        decoded = system.frame_decoder.decode(frame)
        last_frames.insert(0, decoded)

        if len(last_frames) > 90:
            last_frames.remove(last_frames[-1])

    system.comm.transmitter.on_send_frame = store_last_frame

_setup_emulator(system)