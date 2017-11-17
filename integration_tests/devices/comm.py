import datetime
from Queue import Queue, Empty, Full
from threading import Lock

from enum import Enum, unique

import i2cMock
from utils import *


class BeaconFrame(object):
    def __init__(self, payload):
        self._payload = payload
        pass

    def payload(self):
        return self._payload


class DownlinkFrame(object):
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

        num = 0
        num |= bytes[0] << 0
        num |= bytes[1] << 8
        num |= bytes[2] << 16

        apid = num & 0b00111111
        seq = num >> 6
        payload = bytes[3:]

        return DownlinkFrame(apid, seq, payload)

    def __str__(self):
        return 'APID: {} Seq: {} Payload: {}'.format(self._apid, self._seq, self._payload)


class UplinkFrame:
    MAX_PAYLOAD_SIZE = 200 - 5

    def __init__(self, apid, content, security_code=None):
        if security_code is None:
            from build_config import config
            security_code = config['COMM_SECURITY_CODE']
        self._bytes = ensure_byte_list(struct.pack('>L', security_code))
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

    def get_code(self):
        map = {
            self.BaudRate0: 0,
            self.BaudRate1200: 0,
            self.BaudRate2400: 1,
            self.BaudRate4800: 2,
            self.BaudRate9600: 3,
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
    def build(TransmitterCurrentConsumption, ReceiverCurrentConsumption, DopplerOffset, Vcc, OscilatorTemperature,
              AmplifierTemperature, SignalStrength):
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
        super(TransmitterDevice, self).__init__(0x61, "Transmitter")

        # callback called when watchdog is being reset
        # expected prototype:
        # None -> None
        self.on_watchdog_reset = None

        # callback called when hardware reset is being invoked 
        # expected prototype:
        # None -> None
        self.on_hardware_reset = None

        # callback called when hardware reset is being invoked 
        # expected prototype:
        # None -> None
        self.on_reset = None

        # callback called when new frame is being sent
        # expected prototype:
        # byte[] frameContent -> None
        # parameters:
        # frameContent byte array that contains frame content
        self.on_send_frame = None

        # callback called when new transmitter baud rate is being set
        # expected prototype:
        # BaudRate newBaudRate -> BaudRate|None
        # parameters:
        # newBaudRate requested new transmitter baud rate
        # This callback can override the requested baud rate by returning its own the desired value. 
        # Returning none indicates that requested baud rate should be used.
        self.on_set_baudrate = None

        # callback called when transmitter telemetry during last transmission is being requested
        # expected prototype:
        # None -> TransmitterTelemetry|None
        # This callback can override the telemetry retuned by this device by returning the desired response.
        # Returning None will indicate that default telemetry should be reported.
        self.on_get_telemetry_last_transmission = None

        # callback called when transmitter instant telemetry is being requested
        # expected prototype:
        # None -> TransmitterTelemetry|None
        # This callback can override the telemetry retuned by this device by returning the desired response.
        # Returning None will indicate that default telemetry should be reported.
        self.on_get_telemetry_instant = None

        # callback called when beacon is being set
        # expected prototype:
        # None -> Boolean | None
        # This callback can override the beacon set operation by returning information (boolean) whether the 
        # beacon should be set and frame queue emptied (True) of ignored (False)
        self.on_set_beacon = None

        # callback called when idle state is being set
        # callback prototype:
        # bool -> None
        self.on_set_idle_state = None

        # callback called when current state is being requested
        # callback prototype:
        # None -> byte[]
        self.on_report_state = None

        # callback called when uptime is being requested
        # callback prototype:
        # None -> byte[]
        self.on_report_uptime = None

        self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)
        self._lock = Lock()
        self.baud_rate = BaudRate.BaudRate1200
        self.beacon_active = False
        self.transmitter_active = False

        self.current_beacon = None
        self.current_beacon_timestamp = None

        self.last_watchdog_kick = None

    @i2cMock.command([0xAA])
    def _reset(self):
        if call(self.on_reset, None) is None:
            self.reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.last_watchdog_kick = datetime.datetime.now()
        call(self.on_watchdog_reset, None)

    @i2cMock.command([0x10])
    def _send_frame(self, *data):
        self.log.info("Send frame %s", data)

        call(self.on_send_frame, None, self, data)
        with self._lock:
            try:
                self._buffer.put_nowait(data)
            except Full:
                self.log.error("Could not put new item - Queue is full")
            self.beacon_active = False
            return [TransmitterDevice.BUFFER_SIZE - self._buffer.qsize()]

    @i2cMock.command([0x28])
    def _set_baudrate(self, baudrate):
        try:
            baudrate = BaudRate(baudrate)
            self.baud_rate = call(self.on_set_baudrate, baudrate, baudrate)
        except ValueError:
            pass

    @i2cMock.command([0x26])
    def _get_telemetry_last_transmission(self):
        telemetry = call(self.on_get_telemetry_last_transmission, TransmitterTelemetry())
        return telemetry.toArray()

    @i2cMock.command([0x25])
    def _get_telemetry_instant(self):
        telemetry = call(self.on_get_telemetry_instant, TransmitterTelemetry())
        return telemetry.toArray()

    @i2cMock.command([0x14])
    def _set_beacon(self, *data):
        self.log.info("set beacon: %s", data)
        if call(self.on_set_beacon, True):
            self.current_beacon = data[3:]
            self.current_beacon_timestamp = time.localtime()

            self.reset_queue()
            self.beacon_active = True

    @i2cMock.command([0x24])
    def _set_idle_state(self, enabled):
        self.transmitter_active = call(self.on_set_idle_state, enabled, enabled)

    @i2cMock.command([0x40])
    def _report_uptime(self):
        response = call(self.on_report_uptime, None)
        if response is not None:
            return response

        now = datetime.datetime.now()
        return [now.second, now.minute, now.hour, now.day]

    @i2cMock.command([0x41])
    def _report_state(self):
        response = call(self.on_report_state, None)
        if response is not None:
            return response

        response = 0
        if self.beacon_active:
            response |= 1

        if self.transmitter_active:
            response |= 2

        response |= (0x3 & self.baud_rate.get_code()) << 2
        return [response]

    def get_message_from_buffer(self, timeout=None):
        return self._buffer.get(timeout=timeout)

    def reset(self):
        self.reset_queue()

    def reset_queue(self):
        with self._lock:
            self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)

    def queue_size(self):
        return self._buffer.qsize()


class ReceiverDevice(i2cMock.I2CDevice):
    def __init__(self):
        super(ReceiverDevice, self).__init__(0x60, "Receiver")

        # callback called when watchdog is being reset
        # expected prototype:
        # None -> None
        self.on_watchdog_reset = None

        # callback called when hardware reset is being invoked 
        # expected prototype:
        # None -> None
        self.on_hardware_reset = None

        # callback called when hardware reset is being invoked 
        # expected prototype:
        # None -> None
        self.on_reset = None

        # callback called when frame is being removed from the frame buffer
        # expected prototype:
        # ReceiverDevice device -> bool|None
        # parameters:
        # device Reference to the affected device
        # This callback can return bool indicating whether frame should be removed from the buffer.
        # Returning True|None will indicate that frame should be remove from the buffer.
        self.on_frame_remove = None

        # callback called when frame content is being requested
        # expected prototype:
        # ReceiverDevice device -> bool|None
        # parameters:
        # device Reference to the affected device
        # This callback can return bool indicating whether non empty frame should be obtained from the buffer.
        # Returning True|None will indicate that non empty frame should be returned if it is available.
        self.on_frame_receive = None

        # callback called when receiver telemetry is being requested
        # expected prototype:
        # None -> ReceiverTelemetry|None
        # This callback can override the telemetry retuned by this device by returning the desired response.
        # Returning None will indicate that default telemetry should be reported.
        self.on_get_telemetry = None

        # callback called when uptime is being requested
        # callback prototype:
        # None -> byte[]
        self.on_report_uptime = None

        self.last_watchdog_kick = None

        self._buffer = Queue()
        self._lock = Lock()

    @i2cMock.command([0xAA])
    def _reset(self):
        call(self.on_reset, None)
        self.reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.last_watchdog_kick = datetime.datetime.now()
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

    @i2cMock.command([0x40])
    def _report_uptime(self):
        response = call(self.on_report_uptime, None)
        if response is not None:
            return response

        now = datetime.datetime.now()
        return [now.second, now.minute, now.hour, now.day]

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

    def __init__(self, frame_decoder):
        self._frame_decoder = frame_decoder

        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()

        # callback called when hardware reset is being invoked 
        # expected prototype:
        # None -> bool|None
        # This callback can return bool value indicating whether the hardware reset should be performed.
        # Returning True/None indicates that hardware reset should be performed.
        self.on_hardware_reset = None

        # callback called when watchdog is being reset
        # expected prototype:
        # None -> None
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

    def get_frame(self, timeout=None, filter_type=None):
        start = time.time()
        while timeout is None or (time.time() - start < timeout):
            f = None
            try:
                f = self.transmitter.get_message_from_buffer(None if timeout is None else time.time() - start)
            except Empty:
                continue

            frame = self._frame_decoder.decode(f)

            if filter_type is not None:
                if not isinstance(frame, filter_type):
                    continue

            return frame
