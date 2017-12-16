from threading import Thread, Lock

import IPython.lib.guisupport
from datetime import datetime

import zmq

from emulator import Emulator
from emulator.antenna import AntennasModule
from emulator.beacon import BeaconModule
from emulator.comm_zmq_adapter import ZeroMQAdapter
from emulator.eps import EPSModule
from emulator.last_frames import LastFramesModule
from emulator.payload import PayloadModule
from emulator.rtc import RTCModule
from emulator.comm import CommModule
from devices.comm import BeaconFrame
from utils import ensure_byte_list

last_frames = []


class LastBeacon:
    def __init__(self):
        self.time = None
        self.payload = None


last_beacon = LastBeacon()


class CaptureLastBeacon(object):
    def __init__(self, frame_decoder):
        self._last_beacon_lock = Lock()
        self.last_beacon = LastBeacon()

        self._frame_decoder = frame_decoder
        self._catch_beacon_thread = Thread(target=self._catch_beacon)
        self._catch_beacon_thread.daemon = True
        self._incoming_frames = zmq.Context.instance().socket(zmq.SUB)

    def start(self):
        self._incoming_frames.connect("tcp://localhost:7001")
        self._incoming_frames.setsockopt(zmq.SUBSCRIBE, '')

        self._catch_beacon_thread.start()

    def _catch_beacon(self):
        while True:
            frame = ensure_byte_list(self._incoming_frames.recv())

            decoded = self._frame_decoder.decode(frame[16:-2])

            if isinstance(decoded, BeaconFrame):
                self.last_beacon.time = datetime.now()
                self.last_beacon.payload = decoded

            last_frames.insert(0, decoded)

            if len(last_frames) > 90:
                last_frames.remove(last_frames[-1])


def _setup_emulator(system):
    beacon_catcher = CaptureLastBeacon(system.frame_decoder)

    emulator_modules = [
        BeaconModule(system, beacon_catcher.last_beacon),
        AntennasModule(system),
        RTCModule(system),
        EPSModule(system),
        PayloadModule(system),
        CommModule(system, beacon_catcher.last_beacon),
        LastFramesModule(last_frames)
    ]

    emulator = Emulator(emulator_modules)
    emulator.start(IPython.lib.guisupport.get_app_wx())

    emulator.zmq_adapter = ZeroMQAdapter(system.comm)
    beacon_catcher.start()


_setup_emulator(system)
