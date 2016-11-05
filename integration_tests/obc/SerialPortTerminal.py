import logging

import serial


class SerialPortTerminal:
    def __init__(self, comPort, gpio):
        self.log = logging.getLogger("OBCTerm")

        self._gpio = gpio

        self._serial = serial.Serial(comPort, baudrate=2400, timeout=1, rtscts=False)
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

    def command(self, cmd):
        self.waitForPrompt()

        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flushOutput()

        self._serial.write(cmd + "\n")
        self._serial.flush()

        response = self.readUntilPrompt()
        return response.rstrip('\n')

    def reset(self):
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flush()
        self._gpio.high(self._gpio.RESET)
        self._gpio.low(self._gpio.RESET)
        self._gpio.high(self._gpio.RESET)
        self.readUntilPrompt('@')

    def power_off(self):
        self.log.debug("power off")
        self._gpio.low(self._gpio.RESET)

    def power_on(self, clean_state):
        self.log.debug("Powering OBC on")
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flush()
        if clean_state:
            self._gpio.low(self._gpio.CLEAN)
        else:
            self._gpio.high(self._gpio.CLEAN)

        self._gpio.high(self._gpio.RESET)

        self.log.debug("Waiting for OBC to come up")

        self.readUntilPrompt('@')

        self.log.debug("OBC startup done")

    def close(self):
        self._serial.close()