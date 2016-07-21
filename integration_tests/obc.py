import logging

import serial


class SerialPortTerminal:
    def __init__(self, comPort):
        self.log = logging.getLogger("OBCTerm")
        self._serial = serial.Serial(comPort, baudrate=9600, timeout=1, rtscts=False)
        self._serial.rts = False

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
            c = self._serial.read()

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
        self._serial.rts = False
        self._serial.rts = True
        self._serial.rts = False
        self.readUntilPrompt('@')

    def power_off(self):
        self.log.debug("power off")
        self._serial.rts = True

    def power_on(self):
        self.log.debug("Powering OBC on")
        self._serial.reset_input_buffer()
        self._serial.reset_output_buffer()
        self._serial.flushInput()
        self._serial.flush()
        self._serial.rts = False

        self.log.debug("Waiting for OBC to come up")

        self.readUntilPrompt('@')

        self.log.debug("OBC startup done")

    def close(self):
        self._serial.close()


class OBC:
    def __init__(self, terminal):
        self.log = logging.getLogger("OBC")

        self._terminal = terminal
        self._terminal.reset()

    def wait_to_start(self):
        response = self._terminal.command("getState")
        while response != "1":
            response = self._terminal.command("getState")

    def ping(self):
        return self._terminal.command("ping")

    def jump_to_time(self, time):
        self._terminal.command("jumpToTime %d" % time)

    def current_time(self):
        r = self._terminal.command("currentTime")
        return int(r)

    def send_frame(self, data):
        self._terminal.command("sendFrame %s" % data)

    def get_frame_count(self):
        r = self._terminal.command("getFramesCount")
        return int(r)

    def receive_frame(self):
        r = self._terminal.command("receiveFrame")
        return r

    def comm_auto_handling(self, enable):
        if not enable:
            self._terminal.command("pauseComm")

    def reset(self):
        self._terminal.reset()

    def close(self):
        self._terminal.close()

    def power_off(self):
        self._terminal.power_off()

    def power_on(self):
        self._terminal.power_on()

    def list_files(self, path):
        result = self._terminal.command("listFiles %s" % path)
        return result.split('\n')

    def write_file(self, path, content):
        self._terminal.command("writeFile %s %s" % (path, content))

    def read_file(self, path):
        return self._terminal.command("readFile %s" % path)