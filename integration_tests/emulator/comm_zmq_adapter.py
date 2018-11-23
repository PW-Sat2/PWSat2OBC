from Queue import Empty
from threading import Thread
from time import sleep
import numpy as np
import logging

import zmq
import devices
from utils import ensure_string


class ZeroMQAdapter(object):
    def __init__(self, comm, grc_uplink_address="tcp://localhost:7002", grc_downlink_address="tcp://localhost:7003",
                 uplink_per=0, downlink_per=0):
        self._comm = comm  # type: devices.Comm

        self._comm.transmitter.on_send_frame = self._on_downlink_frame

        self._context = zmq.Context.instance()
        self._socket_uplink = self._context.socket(zmq.SUB)
        self._socket_uplink_gnuradio = self._context.socket(zmq.SUB)

        self._downlink_new_msg = self._context.socket(zmq.PUSH)
        self._downlink_delay_msg = self._context.socket(zmq.PULL)
        self._downlink_pub = self._context.socket(zmq.PUB)
        self._downlink_gnuradio_pub = self._context.socket(zmq.PUB)

        self._socket_uplink.bind("tcp://*:%s" % 7000)
        self._socket_uplink_gnuradio.connect(grc_uplink_address)

        self._downlink_new_msg.bind("inproc://downlink/new_msg")
        self._downlink_delay_msg.connect("inproc://downlink/new_msg")

        self._downlink_pub.bind("tcp://*:%s" % 7001)
        self._downlink_gnuradio_pub.connect(grc_downlink_address)

        self._socket_uplink.setsockopt(zmq.SUBSCRIBE, '')
        self._socket_uplink_gnuradio.setsockopt(zmq.SUBSCRIBE, '')

        self._uplink_listener = Thread(target=self._uplink_worker)
        self._uplink_listener.daemon = True
        self._uplink_listener.start()

        self._uplink_gnuradio_listener = Thread(target=self._uplink_gnuradio_worker)
        self._uplink_gnuradio_listener.daemon = True
        self._uplink_gnuradio_listener.start()

        self._downlink_handler = Thread(target=self._downlink_worker)
        self._downlink_handler.daemon = True
        self._downlink_handler.start()
        self._uplink_per = uplink_per
        self._downlink_per = downlink_per

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

    @staticmethod
    def _build_gnuradio_frame(text):
        return ''.join([
            ZeroMQAdapter._build_kiss_header(),
            text
        ])

    def _on_downlink_frame(self, comm, frame):
        self._downlink_new_msg.send(ensure_string(frame))

    def _delay_uplink_frame(self, frame):
        message_sending_time = 8.0 * len(frame) / 1200.0
        sleep(message_sending_time)

    def _delay_downlink_frame(self, frame):
        message_sending_time = 8.0 * len(frame) / float(str(self._comm.transmitter.baud_rate))
        sleep(message_sending_time)

    def _uplink_worker(self):
        while True:
            frame = self._socket_uplink.recv()
            just_content = frame[16:]

            self._delay_uplink_frame(just_content)

            self._comm.receiver.put_frame(just_content)

    def _uplink_gnuradio_worker(self):
        log = logging.getLogger("GNURADIO UPLINK")
        count_all_frames = 0.0
        count_rejected = 0.0

        while True:
            frame = self._socket_uplink_gnuradio.recv()
            just_content = frame[16:]
            count_all_frames+=1

            if self._uplink_per == 0 or np.random.choice(['reject', 'accept'], 1, p=[self._uplink_per, 1 - self._uplink_per])[0] == 'accept':
                log.debug("Uplink frame accepted")
                self._comm.receiver.put_frame(just_content)
            else:
                log.info("Uplink frame dropped because of PER setting")
                count_rejected+=1

            if self._uplink_per != 0:
                log.debug("Current uplink PER = {0}".format(count_rejected/count_all_frames))



    def _downlink_worker(self):
        log = logging.getLogger("GNURADIO DOWNLINK")
        count_all_frames = 0.0
        count_rejected = 0.0

        while True:
            frame = self._downlink_delay_msg.recv()

            count_all_frames+=1

            if self._downlink_per == 0 or np.random.choice(['reject', 'accept'], 1, p=[self._downlink_per, 1 - self._downlink_per])[0] == 'accept':
                log.debug("Downlink frame accepted")
                gnuradio_frame = ZeroMQAdapter._build_gnuradio_frame(frame)
                self._downlink_gnuradio_pub.send(gnuradio_frame)

                kiss_frame = ZeroMQAdapter._build_kiss(frame)

                self._downlink_pub.send(kiss_frame)

                try:
                    self._comm.transmitter.get_message_from_buffer(0)
                except Empty:
                    pass
            else:
                log.info("Downlink frame dropped because of PER setting")
                count_rejected+=1

            if self._downlink_per != 0:
                log.debug("Current downlink PER = {0}".format(count_rejected/count_all_frames))

