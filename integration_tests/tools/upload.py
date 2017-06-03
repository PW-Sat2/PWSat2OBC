import argparse

import serial
import xmodem

parser = argparse.ArgumentParser()

parser.add_argument("port", help="Serial port used to communicate with OBC")
parser.add_argument("file", help="Binary file to upload")
parser.add_argument("index", help="Slot for binary file (0=safe mode, 1-7 applications)")
parser.add_argument("description", help="Description for binary")

args = parser.parse_args()


class Bootloader:
    def __init__(self, port):
        self._port = port

    def wait(self):
        self._wait_for('&')
        self._port.write('S')
        self._wait_for(':')
        self._port.write('\n')
        self._wait_for('#')

    def upload_binary(self, index, description, stream):
        self._port.write('x')

        self._wait_for('Boot Index: ')
        self._port.write(chr(index))

        print 'Boot index selected'

        self._wait_for('Upload Binary: ')

        print 'Uploading binary'
        modem = xmodem.XMODEM(getc=self._xmodem_getc, putc=self._xmodem_putc)
        r = modem.send(stream, quiet=True, callback=self._xmodem_report_progress)

        if not r:
            print 'Upload failed!'
            return False

        print 'Binary uploaded'

        self._wait_for('Boot Description: ')
        self._port.write(description)
        self._port.write('\0\n')

        self._wait_for('Done!')

        print 'Upload finished'

    def _wait_for(self, marker):
        s = ''
        while not s.endswith(marker):
            s += self._port.read(1)

    def _xmodem_getc(self, size, timeout=1):
        d = self._port.read(size)
        return d

    def _xmodem_putc(self, data, timeout=1):
        l = self._port.write(data)
        return l

    def _xmodem_report_progress(self, _, success_count, error_count):
        packet_size = 128

        print 'Transfered: {:.3} KB ({} errors)'.format(success_count * packet_size / 1024.0, error_count)

port = serial.Serial(port=args.port, baudrate=115200)
print args
bootloader = Bootloader(port)

print 'Waiting for bootloader'

bootloader.wait()

print 'Bootloader ready'

with file(args.file, 'rb') as f:
    bootloader.upload_binary(int(args.index), args.description, f)
