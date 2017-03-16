import traceback
from threading import Thread, Event, Lock

import serial
import sys
import binascii
import serial.threaded
import logging

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


class I2CDevice(object):
    def __init__(self, address):
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

        return handler(self, *args)

    def freeze(self):
        self.freeze_end.wait()

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


class I2CMock(object):
    CMD_VERSION = 0x01
    CMD_I2C_WRITE = 0x02
    CMD_I2C_RESPONSE = 0x03
    CMD_I2C_DISABLE = 0x04
    CMD_I2C_ENABLE = 0x05
    CMD_RESTART = 0x06
    CMD_UNLATCH = 0x07
    CMD_STOP = 0x08
    CMD_STOPPED = 0x09

    _port = serial.Serial

    def __init__(self, bus_name, port_name):
        self._log = logging.getLogger("I2C.%s" % bus_name)

        self._port = None
        while self._port is None:
            try:
                self._port = serial.Serial(port=port_name, baudrate=100000, rtscts=True)
            except serial.SerialException as e:
                if not e.args[0].endswith("WindowsError(5, 'Access is denied.')"):
                    raise

        self._freeze_end = Event()

        self._active = False
        self._reader = Thread(target=I2CMock._reader_run, args=(self,))
        self._reader.daemon = True

        self._command_handlers = {
            I2CMock.CMD_VERSION: I2CMock._device_command_version,
            I2CMock.CMD_STOPPED: I2CMock._device_command_stopped,
            I2CMock.CMD_I2C_WRITE: I2CMock._device_command_write
        }

        self._started = Event()
        self._reader_started = Event()
        self._port_lock = Lock()

        self._devices = {}

    def start(self):
        if self._active:
            return

        self._log.debug('Starting DeviceMock')

        if self._freeze_end.is_set():
            self._freeze_end.clear()

        self._reader.start()
        self._reader_started.wait()

        self._command(I2CMock.CMD_RESTART)
        self._log.debug('Waiting for mock to start')
        self._started.wait()
        self._command(I2CMock.CMD_I2C_ENABLE)
        self._active = True

    def add_device(self, device):
        self._devices[device.address] = device

    def stop(self):
        self._log.debug('Requesting stop')
        self._command(I2CMock.CMD_I2C_DISABLE)
        self._command(I2CMock.CMD_STOP)

        self._freeze_end.set()

        self._reader.join()
        self._port.close()
        self._active = False
        self._log.debug('Stopped')

    def unfreeze(self):
        self._freeze_end.set()

    def unlatch(self):
        if self._port.is_open:
            self._command(I2CMock.CMD_UNLATCH)

    def disable(self):
        self._command(I2CMock.CMD_I2C_DISABLE)

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
        self._log.info('Device mock version %d', ord(version))
        self._started.set()

    def _device_command_stopped(self):
        raise DeviceMockStopped()

    def _device_command_write(self, address, *data):
        address = ord(address) / 2
        data = [ord(c) for c in data]

        self._log.info('Device(%X) write(%s)', address, hex_data(data))

        device = self._device(address)
        device.freeze_end = self._freeze_end

        response = device.handle(data) or []

        self._log.debug('Generated response %r', response)

        self._command(I2CMock.CMD_I2C_RESPONSE, response)

        self._log.debug('Response written')

    def _device(self, address):
        if self._devices.has_key(address):
            return self._devices[address]
        else:
            return MissingDevice(address)
