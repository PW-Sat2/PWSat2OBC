import logging
import struct

import serial
import time

from .bootloader import OBCBootloader


class SerialPortTerminal:
    def __init__(self, comPort, gpio):
        self.log = logging.getLogger("OBCTerm")

        self._gpio = gpio

        self._serial = None
        while self._serial is None:
            try:
                self._serial = serial.Serial(comPort, baudrate=115200, timeout=1, rtscts=False)
            except serial.SerialException as e:
                if e.message.find('WindowsError(5,') == -1:
                    raise
        self._gpio.high(self._gpio.RESET)

    def waitForPrompt(self, terminator='>'):
        self._serial.reset_input_buffer()
        self._serial.flushInput()

        self._serial.write("\n")
        self._serial.flush()
        c = self._serial.read(1)
        while c != terminator:
            c = self._serial.read(1)

    def readUntilPrompt(self, terminator='>'):
        data = ""
        c = self._serial.read(1)
        while c != terminator:
            data += c
            c = self._serial.read(1)

        return data

    def _safe_read(self, size):
        data = ''

        while size > 0:
            part = self._serial.read(size)

            data += part
            size -= len(part)

        return data

    def _command_prologue(self, cmd):
        self.waitForPrompt()

        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flushOutput()

        self._serial.write(cmd + "\n")
        self._serial.flush()

    def command(self, cmd):
        self._command_prologue(cmd)

        response = self.readUntilPrompt().rstrip('\n')
        self.log.info("Command " + cmd + " responded with '" + response + "'")
        return response

    def command_no_wait(self, cmd):
        self._command_prologue(cmd)

    def command_with_write_data(self, cmd, data):
        self._command_prologue(cmd)

        self.readUntilPrompt('#')

        self._serial.write(struct.pack('<L', len(data)))
        self._serial.flush()

        remaining = data

        while len(remaining) > 0:
            b = self._safe_read(4)
            part_length = struct.unpack('<L', b)[0]

            part = remaining[0:part_length]

            self._serial.write(part)
            self._serial.flush()

            remaining = remaining[part_length:]

        return self.readUntilPrompt()

    def command_with_read_data(self, cmd):
        self._command_prologue(cmd)
        size = self.readUntilPrompt('\n')
        size = int(size)

        data = self._safe_read(size)

        self.readUntilPrompt()

        return data

    def reset(self, boot_handler):
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flush()
        self._gpio.high(self._gpio.RESET)
        self._gpio.low(self._gpio.RESET)
        self._gpio.high(self._gpio.RESET)

        self._boot(boot_handler)

    def power_off(self):
        self.log.debug("power off")
        self._gpio.low(self._gpio.RESET)

    def power_on(self, boot_handler):
        self.log.debug("Powering OBC on")
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flush()

        self._gpio.high(self._gpio.RESET)

        self.log.debug("Waiting for OBC to come up")

        self._boot(boot_handler)

        self.log.debug("OBC startup done")

    def close(self):
        self._serial.close()

    def wait_for_boot(self, timeout=None):
        return self._boot(timeout)

    def _stay_in_bootloader(self):
        self._serial.write('S')

        c = self._serial.read(1)
        while c != 'O':
            c = self._serial.read(1)

    def _boot(self, boot_handler, timeout=None):
        end = None if timeout is None else time.time() + timeout
        c = ''
        while c != '@':
            c = self._serial.read(1)

            if c == '&':
                self._stay_in_bootloader()
                boot_handler.boot(OBCBootloader(self._serial))

            if end is not None and time.time() > end:
                return False

        return True

