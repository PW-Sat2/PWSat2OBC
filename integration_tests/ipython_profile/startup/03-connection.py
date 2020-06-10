from threading import Thread
from utils import ensure_byte_list, ensure_string
from obc.boot import SelectRunlevel
import zmq


class PowerCycleController(object):
    def __init__(self, system):
        self.system = system

    def on_power_cycle(self):
        self.system.power_off_obc()
        self.system.start()
        self.system.restart([SelectRunlevel(3)])


class ExternalCommunication(object):
    def __init__(self, comm):
        self._comm = comm
        self._comm.transmitter.on_send_frame = self._on_downlink_frame

        self._context = zmq.Context.instance()
        self._socket_uplink = self._context.socket(zmq.SUB)
        self._socket_downlink = self._context.socket(zmq.PUB)

        self._socket_uplink.bind("tcp://*:%s" % 9000)
        self._socket_downlink.bind("tcp://*:%s" % 9001)

        self._socket_uplink.setsockopt(zmq.SUBSCRIBE, '')

        self._uplink_listener = Thread(target=self._uplink_worker)
        self._uplink_listener.daemon = True
        self._uplink_listener.start()

    def _on_downlink_frame(self, comm, frame):
        self._socket_downlink.send(ensure_string(frame))

    def _uplink_worker(self):
        while True:
            frame = self._socket_uplink.recv()
            self._comm.receiver.put_frame(ensure_byte_list(frame))


power_cycle_controller = PowerCycleController(system)

system.eps.controller_b.on_power_cycle = power_cycle_controller.on_power_cycle
system.eps.controller_a.on_power_cycle = power_cycle_controller.on_power_cycle

external_comm = ExternalCommunication(system.comm)

power_cycle_controller.on_power_cycle()
