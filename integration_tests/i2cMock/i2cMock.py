import traceback
from idlelib.PyShell import PORT
from threading import Thread, Event, Lock

import serial
import sys
import binascii
import serial.threaded
import logging

from enum import IntEnum

from utils import hex_data

DEVICE_SELECTED_FOR_WRITE = 'W'
DEVICE_SELECTED_FOR_READ = 'R'
BYTE_WRITTEN = 'B'
WRITE_FINISHED = 'S'
NEXT_BYTE = 'N'
READ_FINISHED = 'F'
CONTROL_CHAR = 'C'
CONTROL_CMD_RESTART = 'R'


def command(bytes):
    def wrapper(func):
        func.bytes = bytes
        return func

    return wrapper


class LatchBusError(Exception):
    pass


class I2CDevice(object):
    def __init__(self, address):
        if address >= 0x80:
            raise Exception("I2C address cannot be longer than 7 bits. Got address=%X" % address)

        self.address = address
        self.handlers = self._init_handlers()
        self.response = None
        self.freeze_end = None

    def handle(self, data):
        self.response = None

        handler = None
        args = []
        for h in self.handlers:
            if h.bytes == data[:len(h.bytes)]:
                handler = h
                args = data[len(h.bytes):]

        if handler is None:
            return self._missing_handler(data)

        self.response = handler(self, *args) or []

    def get_response(self):
        return self.response

    def latch(self):
        raise LatchBusError()

    def _missing_handler(self, data):
        logging.getLogger('Device: 0x{:2X}'.format(self.address)).error('Missing handler for 0x{:2X}'.format(self.address, binascii.hexlify(bytearray(data))))

    def _init_handlers(self):
        handlers = []
        for v in self.__class__.__dict__.itervalues():
            try:
                if v.bytes is not None:
                    handlers.append(v)
            except AttributeError:
                pass

        return handlers


class MissingDevice(I2CDevice):
    def __init__(self, address):
        super(MissingDevice, self).__init__(address)
        self._log = logging.getLogger('MissingDevice: 0x{:2X}'.format(address))

    @command([])
    def catch_all(self, *data):
        self._log.error('Missing handler for 0x{}'.format(binascii.hexlify(bytearray(data))))
        return [0xCC]


class DeviceMockStopped(Exception):
    pass


PortA = 0 << 4
PortB = 1 << 4
PortC = 2 << 4
PortD = 3 << 4
PortF = 4 << 4


class MockPin(IntEnum):
    PA0 = PortA | 0
    PA1 = PortA | 1
    PA4 = PortA | 4
    PA6 = PortA | 6
    PA7 = PortA | 7
    PA8 = PortA | 8
    PA9 = PortA | 9
    PA10 = PortA | 10
    PA11 = PortA | 11
    PA12 = PortA | 12
    PA13 = PortA | 13
    PA15 = PortA | 15
    PB0 = PortB | 0
    PB1 = PortB | 1
    PB2 = PortB | 2
    PB3 = PortB | 3
    PB4 = PortB | 4
    PB5 = PortB | 5
    PB6 = PortB | 6
    PB7 = PortB | 7
    PB12 = PortB | 12
    PB13 = PortB | 13
    PB14 = PortB | 14
    PB15 = PortB | 15
    PC0 = PortC | 0
    PC1 = PortC | 1
    PC2 = PortC | 2
    PC3 = PortC | 3
    PC4 = PortC | 4
    PC7 = PortC | 7
    PC9 = PortC | 9
    PC10 = PortC | 10
    PC11 = PortC | 11
    PC12 = PortC | 12
    PC13 = PortC | 13
    PD2 = PortD | 2
    PF1 = PortF | 1
    PF4 = PortF | 4
    PF5 = PortF | 5
    PF6 = PortF | 6
    PF7 = PortF | 7

class UnsupportedMockVersion(Exception):
    def __init__(self, version):
        super(UnsupportedMockVersion, self).__init__("Unsupported mock version %d" % version)


class I2CMock(object):
    CMD_BUS_MASK = 0b01000000
    CMD_PLD_MASK = 0b10000000

    CMD_VERSION = 0x01
    CMD_RESTART = 0x06
    CMD_STOP = 0x08
    CMD_STOPPED = 0x09
    CMD_GPIO_LOW = 0xA1
    CMD_GPIO_HIGH = 0xA2

    CMD_I2C_BUS_WRITE = CMD_BUS_MASK | 0x02
    CMD_I2C_BUS_RESPONSE = CMD_BUS_MASK | 0x03
    CMD_I2C_BUS_DISABLE = CMD_BUS_MASK | 0x04
    CMD_I2C_BUS_ENABLE = CMD_BUS_MASK | 0x05
    CMD_I2C_BUS_UNLATCH = CMD_BUS_MASK | 0x07
    CMD_I2C_BUS_REQUEST_RESPONSE = CMD_BUS_MASK | 0xA
    CMD_I2C_BUS_ENABLE_DEVICES = CMD_BUS_MASK | 0xB
    CMD_I2C_BUS_LATCH = CMD_BUS_MASK | 0xC

    CMD_I2C_PLD_WRITE = CMD_PLD_MASK | 0x02
    CMD_I2C_PLD_RESPONSE = CMD_PLD_MASK | 0x03
    CMD_I2C_PLD_DISABLE = CMD_PLD_MASK | 0x04
    CMD_I2C_PLD_ENABLE = CMD_PLD_MASK | 0x05
    CMD_I2C_PLD_UNLATCH = CMD_PLD_MASK | 0x07
    CMD_I2C_PLD_REQUEST_RESPONSE = CMD_PLD_MASK | 0xA
    CMD_I2C_PLD_ENABLE_DEVICES = CMD_PLD_MASK | 0xB
    CMD_I2C_PLD_LATCH = CMD_PLD_MASK | 0xC

    _port = serial.Serial

    def __init__(self, port_name, baudrate=100000, rtscts=False):
        self._log = logging.getLogger("I2C")
        self._bus_log = logging.getLogger("I2C.BUS")
        self._pld_log = logging.getLogger("I2C.PLD")

        self._port = None
        while self._port is None:
            try:
                self._port = serial.Serial(port=port_name, baudrate=baudrate, rtscts=rtscts)
            except serial.SerialException as e:
                if not e.args[0].endswith("WindowsError(5, 'Access is denied.')"):
                    raise

        self._active = False
        self._reader = Thread(target=I2CMock._reader_run, args=(self,))
        self._reader.daemon = True

        self._command_handlers = {
            I2CMock.CMD_VERSION: I2CMock._device_command_version,
            I2CMock.CMD_STOPPED: I2CMock._device_command_stopped,

            I2CMock.CMD_I2C_BUS_WRITE: I2CMock._device_command_bus_write,
            I2CMock.CMD_I2C_BUS_REQUEST_RESPONSE: I2CMock._device_command_bus_request_response,

            I2CMock.CMD_I2C_PLD_WRITE: I2CMock._device_command_pld_write,
            I2CMock.CMD_I2C_PLD_REQUEST_RESPONSE: I2CMock._device_command_pld_request_response
        }

        self._started = Event()
        self._reader_started = Event()
        self._port_lock = Lock()

        self._bus_devices = {}
        self._pld_devices = {}

        self._version = None

    def start(self):
        if self._active:
            return

        self._log.debug('Starting DeviceMock')

        self._reader.start()
        self._reader_started.wait()

        self._command(I2CMock.CMD_RESTART)
        self._log.debug('Waiting for mock to start')
        self._started.wait()

        self.enable_bus_devices(self._bus_devices.keys(), True)
        self.enable_pld_devices(self._pld_devices.keys(), True)

        self.enable_bus()
        self.enable_payload()
        self._active = True

    def add_bus_device(self, device):
        self._bus_devices[device.address] = device

    def add_pld_device(self, device):
        self._pld_devices[device.address] = device

    def enable_bus_devices(self, devices, enabled):
        mask = enabled << 7
        args = map(lambda x: x | mask, devices)

        self._command(I2CMock.CMD_I2C_BUS_ENABLE_DEVICES, args)

    def enable_pld_devices(self, devices, enabled):
        mask = enabled << 7
        args = map(lambda x: x | mask, devices)

        self._command(I2CMock.CMD_I2C_PLD_ENABLE_DEVICES, args)

    def stop(self):
        self._log.debug('Requesting stop')
        self.disable()
        self._command(I2CMock.CMD_STOP)

        self._reader.join()
        self._port.close()
        self._active = False
        self._log.debug('Stopped')

    def unlatch(self):
        if self._port.is_open:
            self._command(I2CMock.CMD_I2C_BUS_UNLATCH)

    def disable(self):
        self.disable_bus()
        self.disable_payload()

    def disable_bus(self):
        self._command(I2CMock.CMD_I2C_BUS_DISABLE)

    def disable_payload(self):
        self._command(I2CMock.CMD_I2C_PLD_DISABLE)

    def enable_bus(self):
        self._command(I2CMock.CMD_I2C_BUS_ENABLE)

    def enable_payload(self):
        self._command(I2CMock.CMD_I2C_BUS_ENABLE)

    def gpio_low(self, pin):
        self._command(I2CMock.CMD_GPIO_LOW, [int(pin)])

    def gpio_high(self, pin):
        self._command(I2CMock.CMD_GPIO_HIGH, [int(pin)])

    def _command(self, cmd, data=[]):
        raw = ['S', cmd]
        size = [len(data)]
        raw.extend(self._escape(size))
        raw.extend(self._escape(data))

        with self._port_lock:
            self._port.write(raw)

    def _escape(self, data):
        def escape_char(c):
            if c == 'S' or c == ord('S'):
                return ['S', 'S']
            else:
                return [c]

        escaped = map(escape_char, data)

        return [x for y in escaped for x in y]

    def _reader_run(self):
        self._log.debug("Worker thread starting")

        self._port.reset_input_buffer()
        self._port.reset_output_buffer()

        self._reader_started.set()

        try:
            while self._port.is_open:
                (cmd, data) = self._read_command()

                self._log.debug("Received command %X (%s)", cmd, hex_data(data))

                self._command_handlers[cmd](self, *data)
        except DeviceMockStopped:
            pass
        except Exception as e:
            self._log.error("Handle command exception %s", traceback.format_exc(e))

        self._log.debug("Finished worker thread")

    def _read_command(self):
        preamble = self._port.read(2)
        cmd = ord(preamble[0])
        length = ord(preamble[1])

        data = []
        if length > 0:
            data = self._port.read(length)

        return cmd, data

    def _device_command_version(self, version):
        self._version = ord(version)

        self._log.info('Device mock version %d', self._version)
        self._started.set()

        if self._version == 2:
            self._log.warn('Using DeviceMock v2 compatibility')
        elif self._version == 4:
            self._log.warn('Using DeviceMock v4')
        elif self._version != 3:
            raise UnsupportedMockVersion(self._version)

    def _device_command_stopped(self):
        raise DeviceMockStopped()

    def _device_command_pld_write(self, address, *data):
        address = ord(address)
        data = [ord(c) for c in data]

        self._pld_log.info('Device(%X) write(%s)', address, hex_data(data))

        try:
            device = self._pld_device(address)

            device.handle(data)

            response = device.get_response()

            self._pld_log.debug('Generated response %r', response)
        except LatchBusError:
            self._bus_log.warn('Device request latching bus')
            self._command(I2CMock.CMD_I2C_PLD_LATCH)

    def _device_command_pld_request_response(self, address):
        address = ord(address)

        self._pld_log.info('Device(%X) read', address)
        device = self._pld_device(address)

        response = device.get_response() or []

        self._command(I2CMock.CMD_I2C_PLD_RESPONSE, response)

        self._pld_log.debug('Response written (%r)', response)

    def _device_command_bus_write(self, address, *data):
        address = ord(address)
        data = [ord(c) for c in data]

        self._bus_log.info('Device(%X) write(%s)', address, hex_data(data))
        try:
            device = self._bus_device(address)

            device.handle(data)

            response = device.get_response()

            self._bus_log.debug('Generated response %r', response)
        except LatchBusError:
            self._bus_log.warn('Device request latching bus')
            self._command(I2CMock.CMD_I2C_BUS_LATCH)

    def _device_command_bus_request_response(self, address):
        address = ord(address)

        self._bus_log.info('Device(%X) read', address)
        device = self._bus_device(address)

        response = device.get_response() or []

        self._command(I2CMock.CMD_I2C_BUS_RESPONSE, response)

        self._bus_log.debug('Response written (%r)', response)

    def _bus_device(self, address):
        if self._bus_devices.has_key(address / 2):
            return self._bus_devices[address / 2]
        else:
            return MissingDevice(address)

    def _pld_device(self, address):
        if self._pld_devices.has_key(address / 2):
            return self._pld_devices[address / 2]
        else:
            return MissingDevice(address)
