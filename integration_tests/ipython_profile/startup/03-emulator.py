import IPython.lib.guisupport

from emulator import Emulator
from emulator.antenna import AntennasModule
from emulator.beacon import BeaconModule
from emulator.eps import EPSModule
from emulator.last_frames import LastFramesModule
from emulator.payload import PayloadModule
from emulator.rtc import RTCModule
from emulator.comm import CommModule
from devices.comm import BeaconFrame
from time import time

last_frames = []


class LastBeacon:
    def __init__(self):
        self.time = None
        self.payload = None

last_beacon = LastBeacon()

def _setup_emulator(system):
    emulator_modules = [
        BeaconModule(last_beacon, system),
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
        if isinstance(decoded, BeaconFrame):
            last_beacon.time = time()
            last_beacon.payload = decoded.payload()
        last_frames.insert(0, decoded)

        if len(last_frames) > 90:
            last_frames.remove(last_frames[-1])

    system.comm.transmitter.on_send_frame = store_last_frame

_setup_emulator(system)