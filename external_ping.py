# This script requires pyzmq: https://pypi.org/project/pyzmq/
# Usage:
# python external_ping.py <server_address>

from typing import Any, List
from threading import Thread
import sys
import time

import zmq


class ExternalCommunication:
    def __init__(self, address: str) -> None:
        self._context = zmq.Context.instance()
        self._socket_uplink = self._context.socket(zmq.PUB)
        self._socket_downlink = self._context.socket(zmq.SUB)

        self._socket_uplink.connect(f'tcp://{address}:9000')
        self._socket_downlink.connect(f'tcp://{address}:9001')

        self._socket_downlink.setsockopt(zmq.SUBSCRIBE, b'')

        self._downlink_listener = Thread(target=self._downlink_worker)
        self._downlink_listener.daemon = True
        self._downlink_listener.start()

    def send_frame(self, frame: List[int]) -> None:
        self._socket_uplink.send(bytes(frame))

    def _downlink_worker(self) -> None:
        while True:
            frame = self._socket_downlink.recv()

            print(f'Frame received: [{", ".join([hex(b) for b in frame])}]')


comm = ExternalCommunication(sys.argv[1])

time.sleep(5) # Wait for zmq connection

ping_frame = [
    0xBB, 0xCC, 0xDD, 0xEE, # Security code
    0x50 # PID
]

comm.send_frame(ping_frame)

time.sleep(5) # Wait for reply
