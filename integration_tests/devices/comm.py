import logging
import struct

import i2cMock
import time
from enum import Enum, unique
from Queue import Queue, Empty
from threading import Lock
from utils import *
from build_config import config

class DownlinkFrame:
    def __init__(self, apid, seq, payload):
        self._apid = apid
        self._seq = seq
        self._payload = payload

    def apid(self):
        return self._apid

    def seq(self):
        return self._seq

    def payload(self):
        return self._payload

    @classmethod
    def parse(cls, bytes):
        bytes = ensure_byte_list(bytes)

        apid = (bytes[0] & 0b11111100) >> 2
        seq = ((bytes[0] & 0b11) << 16) | (bytes[1] << 8) | (bytes[2])
        payload = bytes[3:]

        return DownlinkFrame(apid, seq, payload)


class UplinkFrame:
    def __init__(self, apid, content, security_code=config['COMM_SECURITY_CODE']):
        self._bytes = ensure_byte_list(struct.pack('>L',security_code))
        self._bytes += [apid]
        self._bytes += ensure_byte_list(content)

    def build(self):
        return self._bytes

@unique
class BaudRate(Enum):
    BaudRate0 = 0
    BaudRate1200 = 1
    BaudRate2400 = 2
    BaudRate4800 = 4
    BaudRate9600 = 8

    def __str__(self):
        map = {
            self.BaudRate0: "0",
            self.BaudRate1200: "1200",
            self.BaudRate2400: "2400",
            self.BaudRate4800: "4800",
            self.BaudRate9600: "9600",
        }

        return map[self]

class TransmitterTelemetry(object):
    def __init__(self):
        self.RFReflectedPower = 0
        self.AmplifierTemperature = 0
        self.RFForwardPower = 0
        self.TransmitterCurrentConsumption = 0

    @staticmethod
    def build(rfPower, temperature, forwardPower, current):
        telemetry = TransmitterTelemetry()
        telemetry.RFReflectedPower = rfPower
        telemetry.AmplifierTemperature = temperature
        telemetry.RFForwardPower = forwardPower
        telemetry.TransmitterCurrentConsumption = current
        return telemetry

    def toArray(self):
        return [
            lower_byte(self.RFReflectedPower), higher_byte(self.RFReflectedPower),
            lower_byte(self.AmplifierTemperature), higher_byte(self.AmplifierTemperature),
            lower_byte(self.RFForwardPower), higher_byte(self.RFForwardPower),
            lower_byte(self.TransmitterCurrentConsumption), higher_byte(self.TransmitterCurrentConsumption)
            ]

class ReceiverTelemetry(object):
    def __init__(self):
        self.TransmitterCurrentConsumption = 0
        self.ReceiverCurrentConsumption = 0
        self.DopplerOffset = 0
        self.Vcc = 0
        self.OscilatorTemperature = 0
        self.AmplifierTemperature = 0
        self.SignalStrength = 0

    @staticmethod
    def build(TransmitterCurrentConsumption, ReceiverCurrentConsumption, DopplerOffset, Vcc, OscilatorTemperature, AmplifierTemperature, SignalStrength):
        telemetry = ReceiverTelemetry()
        telemetry.TransmitterCurrentConsumption = TransmitterCurrentConsumption
        telemetry.ReceiverCurrentConsumption = ReceiverCurrentConsumption
        telemetry.DopplerOffset = DopplerOffset
        telemetry.Vcc = Vcc
        telemetry.OscilatorTemperature = OscilatorTemperature
        telemetry.AmplifierTemperature = AmplifierTemperature
        telemetry.SignalStrength = SignalStrength
        return telemetry

    def toArray(self):
        return [
            lower_byte(self.TransmitterCurrentConsumption), higher_byte(self.TransmitterCurrentConsumption),
            lower_byte(self.DopplerOffset), higher_byte(self.DopplerOffset),
            lower_byte(self.ReceiverCurrentConsumption), higher_byte(self.ReceiverCurrentConsumption),
            lower_byte(self.Vcc), higher_byte(self.Vcc),
            lower_byte(self.OscilatorTemperature), higher_byte(self.OscilatorTemperature),
            lower_byte(self.AmplifierTemperature), higher_byte(self.AmplifierTemperature),
            lower_byte(self.SignalStrength), higher_byte(self.SignalStrength),
            ]

class TransmitterDevice(i2cMock.I2CDevice):
    BUFFER_SIZE = 40

    def __init__(self):
        super(TransmitterDevice, self).__init__(0x62)
        self.log = logging.getLogger("Comm Transmitter")
        self.on_watchdog_reset = None
        self.on_hardware_reset = None
        self.on_reset = None
        self.on_send_frame = None
        self.on_set_baudrate = None
        self.on_get_telemetry = None
        self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)
        self._lock = Lock()
        self.baud_rate = BaudRate.BaudRate1200
    
    @i2cMock.command([0xAA])
    def _reset(self):
        self.log.info("Reset")
        call(self.on_reset, None)
        self.reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        self.log.info("Hardware Reset")
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.log.info("Watchdog Reset")
        call(self.on_watchdog_reset, None)

    @i2cMock.command([0x10])
    def _send_frame(self, *data):
        self.log.info("Send frame %s", data)

        call(self.on_send_frame, None, self, data)
        with self._lock:
            self._buffer.put_nowait(data)
            return [TransmitterDevice.BUFFER_SIZE - self._buffer.qsize()]

    @i2cMock.command([0x28])
    def _set_baudrate(self, baudrate):
        baudrate = BaudRate(baudrate)
        self.baud_rate = call(self.on_set_baudrate, baudrate, baudrate)

    @i2cMock.command([0x26])
    def _get_telemetry(self):
        telemetry = call(self.on_get_telemetry, TransmitterTelemetry())
        return telemetry.toArray()

    def get_message_from_buffer(self, timeout=None):
        return self._buffer.get(timeout=timeout)

    def reset(self):
        with self._lock:
            self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)


class ReceiverDevice(i2cMock.I2CDevice):
    def __init__(self):
        super(ReceiverDevice, self).__init__(0x60)
        self.log = logging.getLogger("Comm Receiver")
        self.on_watchdog_reset = None
        self.on_hardware_reset = None
        self.on_reset = None
        self.on_frame_remove = None
        self.on_frame_receive = None
        self.on_get_telemetry = None
        self._buffer = Queue()
        self._lock = Lock()

    @i2cMock.command([0xAA])
    def _reset(self):
        self.log.info("Reset")
        call(self.on_reset, None)
        self.reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        self.log.info("Hardware reset")
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.log.info("Watchdog Reset")
        call(self.on_watchdog_reset, None)

    @i2cMock.command([0x21])
    def _get_number_of_frames(self):
        with self._lock:
            size = self._buffer.qsize();
            return [size & 0xff, size / 256]

    @i2cMock.command([0x22])
    def _receive_frame(self):
        if call(self.on_frame_receive, True, self):
            with self._lock:
                if self._buffer.empty():
                    return []

                frame = self._buffer.queue[0]
                return ReceiverDevice.build_frame_response(frame, 257, 300)
        else:
            return []

    @i2cMock.command([0x24])
    def _remove_frame(self):
        try:
            if call(self.on_frame_remove, True, self):
                with self._lock:
                    self._buffer.get_nowait()
        except Empty:
            pass

    @i2cMock.command([0x1A])
    def _get_telemetry(self):
        telemetry = call(self.on_get_telemetry, ReceiverTelemetry())
        return telemetry.toArray()

    def reset(self):
        with self._lock:
            self._buffer = Queue()

    @classmethod
    def build_frame_response(cls, content, doppler, rssi):
        length = len(content)

        length_bytes = [length & 0xFF, (length >> 8) & 0xFF]
        doppler_bytes = [doppler & 0xFF, (doppler >> 8) & 0xFF]
        rssi_bytes = [rssi & 0xFF, (rssi >> 8) & 0xFF]

        content_bytes = ensure_byte_list(content)

        return length_bytes + doppler_bytes + rssi_bytes + content_bytes

    def put_frame(self, data):
        with self._lock:
            self._buffer.put_nowait(data)

    def queue_size(self):
        with self._lock:
            return self._buffer.qsize()


class Comm(object):
    MAX_UPLINK_FRAME_SIZE = 200

    def __init__(self):
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()
        self.on_hardware_reset = None
        self.on_watchdog_reset = None
        self.transmitter.on_hardware_reset = self._hwreset
        self.transmitter.on_watchdog_reset = self._watchdog_reset
        self.receiver.on_hardware_reset = self._hwreset
        self.receiver.on_watchdog_reset = self._watchdog_reset

    def _hwreset(self):
        if call(self.on_hardware_reset, True, self):
            self.transmitter.reset()
            self.receiver.reset()

    def _watchdog_reset(self):
        call(self.on_watchdog_reset, None, self)

    def put_frame(self, frame):
        self.receiver.put_frame(frame.build())

    def get_frame(self, timeout=None):
        f = self.transmitter.get_message_from_buffer(timeout)

        return DownlinkFrame.parse(f)
