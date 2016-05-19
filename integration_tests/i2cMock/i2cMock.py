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

        if handler == None:
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

    def __init__(self, portName, baudrate=1000000):
        self.port = serial.Serial(port=portName, baudrate=baudrate, rtscts=True)
        self.active = False
        self.reader = Thread(target=I2CMock._run, args=(self,))
        self.reader.daemon = True

    def start(self):
        self.reader.start()

    def add_device(self, device):
        self.devices[device.address] = device

    def stop(self):
        self.active = False

    def close(self):
        self.port.close()
        self.reader.join()
        self.port = None

    def _run(self):
        self.log.debug("Worker thread starting")
        while self.port.is_open:
            try:
                c = self.port.read(1)

                self.log.debug("Received data: %s", c)

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
            self.log.warning("Unknown mock command %s", cmd)
            sys.stdout.write(cmd)

    def _device(self, address):
        if self.devices.has_key(address):
            return self.devices[address]
        else:
            return MissingDevice(address)

    def _device_selected_for_write(self):
        address = ord(self.port.read(1))

        self.log.debug("Device %d selected for write", address)

        numberOfBytes = ord(self.port.read(1))

        self.log.debug("Number of bytes to write: %d", numberOfBytes)

        data = [ord(self.port.read()) for _ in range(numberOfBytes)]

        self.log.debug("Written bytes: %s", data)

        device = self._device(address)

        device.handle(data)

    def _device_selected_for_read(self):
        address = ord(self.port.read(1))

        self.log.debug("Device %d selected for read", address)

        device = self._device(address)

        data = device.read()

        self.port.write([len(data)] + data)
