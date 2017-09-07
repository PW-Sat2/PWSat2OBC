import argparse
import os

import math
import serial
import xmodem
import progressbar

parser = argparse.ArgumentParser()

parser.add_argument("port", help="Serial port used to communicate with OBC")
parser.add_argument("file", help="Binary file to upload")
parser.add_argument("description", help="Description for binary")
parser.add_argument("index", help="Slot for binary file (0-5)", nargs='+')

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
        # type: (int, str, file) -> bool

        self._port.write('x')

        self._wait_for('Boot Index: ')
        self._port.write(str(index))

        self._wait_for('Upload Binary: ')

        widgets = [
            'Uploading to slot %d ' % index, progressbar.Percentage(),
            ' ', progressbar.Bar(marker='#', left='[', right=']'),
            ' ', progressbar.ETA(),
            ' ', progressbar.FileTransferSpeed(),
        ]

        file_size = os.fstat(f.fileno()).st_size

        with progressbar.ProgressBar(widgets=widgets, max_value=file_size) as bar:
            modem = xmodem.XMODEM(getc=self._xmodem_getc, putc=self._xmodem_putc)
            r = modem.send(stream, quiet=True, callback=self._xmodem_report_progress(bar, file_size))

        if not r:
            print 'Upload failed!'
            return False

        self._wait_for('Boot Description: ')
        self._port.write(description)
        self._port.write('\0\n')

        self._wait_for('Done!')

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

    def _xmodem_report_progress(self, bar, file_size):
        # type: (progressbar.ProgressBar, int) -> function
        def report(_, success_count, error_count):
            packet_size = 128

            transferred_size = min([file_size, packet_size * success_count])

            bar.update(transferred_size)

            # print 'Transfered: {:.3} KB ({} errors)'.format(success_count * packet_size / 1024.0, error_count)
        return report


def verify_correct_format(file):
    header = file.read(4)
    if header[0] == ':':
        print "Selected HEX file, you have to select BIN file"
        return False
    elif 'ELF' in header:
        print "Selected ELF file, you have to select BIN file"
        return False
    file.seek(0)
    return True

with file(args.file, 'rb') as f:
    if not verify_correct_format(f):
        exit(1)

port = serial.Serial(port=args.port, baudrate=115200)
print args
bootloader = Bootloader(port)

print 'Waiting for bootloader'

bootloader.wait()

print 'Bootloader ready'

for slot in args.index:
    with file(args.file, 'rb') as f:
        bootloader.upload_binary(int(slot), args.description, f)
