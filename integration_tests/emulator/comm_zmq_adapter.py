from threading import Thread
import zmq
import devices
from utils import ensure_string


class ZeroMQAdapter(object):
    def __init__(self, comm):
        self._comm = comm  # type: devices.Comm

        self._comm.transmitter.on_send_frame = self._on_downlink_frame

        self._context = zmq.Context.instance()
        self._socket_uplink = self._context.socket(zmq.SUB)

        self._downlink_new_msg = self._context.socket(zmq.PUSH)
        self._downlink_delay_msg = self._context.socket(zmq.PULL)
        self._downlink_pub = self._context.socket(zmq.PUB)

        self._socket_uplink.bind("tcp://*:%s" % 7000)

        self._downlink_new_msg.bind("inproc://downlink/new_msg")
        self._downlink_delay_msg.connect("inproc://downlink/new_msg")

        self._downlink_pub.bind("tcp://*:%s" % 7001)

        self._socket_uplink.setsockopt(zmq.SUBSCRIBE, '')

        self._uplink_listener = Thread(target=self._uplink_worker)
        self._uplink_listener.daemon = True
        self._uplink_listener.start()

        self._downlink_handler = Thread(target=self._downlink_worker)
        self._downlink_handler.daemon = True
        self._downlink_handler.start()

    @staticmethod
    def _encode_callsign(call):
        return ''.join([chr(ord(i) << 1) for i in call])

    @staticmethod
    def _build_kiss_header():
        return ''.join([
            ZeroMQAdapter._encode_callsign('PWSAT2'),
            chr(96),
            ZeroMQAdapter._encode_callsign('PWSAT2'),
            chr(97),
            chr(3),
            chr(0xF0)
        ])

    @staticmethod
    def _build_kiss(text):
        return ''.join([
            ZeroMQAdapter._build_kiss_header(),
            text,
            '\x00\x00'
        ])

    def _on_downlink_frame(self, comm, frame):
        self._downlink_new_msg.send(ensure_string(frame))

    def _delay_uplink_frame(self, frame):
        pass

    def _delay_downlink_frame(self, frame):
        pass

    def _uplink_worker(self):
        while True:
            frame = self._socket_uplink.recv()
            just_content = frame[16:]

            self._delay_uplink_frame(just_content)

            self._comm.receiver.put_frame(just_content)

    def _downlink_worker(self):
        while True:
            frame = self._downlink_delay_msg.recv()

            self._delay_downlink_frame(frame)

            kiss_frame = ZeroMQAdapter._build_kiss(frame)

            self._downlink_pub.send(kiss_frame)
            self._comm.transmitter.get_message_from_buffer(0)