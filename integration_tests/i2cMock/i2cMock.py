from threading import Thread

import serial
import sys
import binascii
import serial.threaded
import logging

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
    address = None
    handlers = []
    response = None

    def __init__(self, address):
        self.address = address
        self.handlers = self._init_handlers()

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

        self.response = handler(self, *args)

    def read(self):
        if self.response is None:
            return []
        else:
            return self.response

    def _missing_handler(self, data):
        print 'Device {:2X}: missing handler for {}'.format(self.address, binascii.hexlify(bytearray(data)))

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
        return super(MissingDevice, self).__init__(address)

    @command([])
    def catch_all(self, *data):
        print 'Missing device({}).command({})'.format(self.address, binascii.hexlify(bytearray(data)))
        return [0xCC]


class I2CMock(object):
    log = logging.getLogger("I2CMock")

    port = serial.Serial
    devices = {}

    def __init__(self, port_name, baudrate=250000):
        self.port = serial.Serial(port=port_name, baudrate=baudrate, rtscts=True, write_timeout=None)
        self.active = False
        self.reader = Thread(target=I2CMock._run, args=(self,))
        self.reader.daemon = True

    def start(self):
        self.port.reset_input_buffer()
        self.port.reset_output_buffer()

        self.restart()

        self.reader.start()

    def add_device(self, device):
        self.devices[device.address] = device

    def stop(self):
        self.active = False

    def close(self):
        self.port.close()
        self.reader.join()
        self.port = None

    def restart(self):
        self.port.write(CONTROL_CHAR + CONTROL_CMD_RESTART)

        c = self.port.read(1)
        while c != 'X':
            c = self.port.read(1)

    def _run(self):
        self.log.debug("Worker thread starting")
        while self.port.is_open:
            try:
                c = self.port.read(1)

                self.handle_command(c)
            except serial.SerialException as e:
                self.log.error("Serial read exception %r. Breaking", e)
                break
            except Exception as e:
                self.log.error("Serial read exception %r", e)

        self.log.debug("Finished worker thread")

    def handle_command(self, cmd):
        if cmd == DEVICE_SELECTED_FOR_WRITE:
            self._device_selected_for_write()
        elif cmd == DEVICE_SELECTED_FOR_READ:
            self._device_selected_for_read()
        else:
            self.log.warning("%s", cmd)
            sys.stdout.write(cmd)

    def _device(self, address):
        if self.devices.has_key(address):
            return self.devices[address]
        else:
            return MissingDevice(address)

    def _device_selected_for_write(self):
        address = ord(self.port.read(1))

        self.log.debug("Device %d selected for write", address)

        number_of_bytes = ord(self.port.read(1))

        self.log.debug("Number of bytes to write: %d", number_of_bytes)

        data = [ord(self.port.read()) for _ in range(number_of_bytes)]

        self.log.debug("Written bytes: %s", data)

        device = self._device(address)

        device.handle(data)

    def _escape(self, data):
        def escape_char(c):
            if c == CONTROL_CHAR or c == ord(CONTROL_CHAR):
                return [CONTROL_CHAR, CONTROL_CHAR]
            else:
                return [c]

        escaped = map(escape_char, data)

        return [x for y in escaped for x in y]

    def _device_selected_for_read(self):
        address = ord(self.port.read(1))

        self.log.debug("Device %d selected for read", address)

        device = self._device(address)

        data = device.read()

        self.log.debug("Response bytes: %s", data)

        response = [len(data)] + data
        escaped = self._escape(response)

        self.port.write(escaped)
        self.port.reset_output_buffer()

        self.log.debug("Bytes written")
