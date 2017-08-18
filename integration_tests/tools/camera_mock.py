import struct
from threading import Thread
import sys
import os
import serial
import logging
import time
from enum import IntEnum
import argparse

try:
    from devices import PhotoResolution
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from devices import PhotoResolution
from utils import ensure_byte_list, call

logging.basicConfig(level=logging.DEBUG)


class CommandCode(IntEnum):
    Initial = 0x01
    GetPicture = 0x04
    Snapshot = 0x05
    SetPackageSize = 0x06
    SetBaudRate = 0x07
    Reset = 0x08
    Data = 0x0A
    Sync = 0x0D
    ACK = 0x0E
    NAK = 0x0F
    Light = 0x13


class CameraMock(object):
    def __init__(self, port_name):
        self._port = serial.Serial(port=port_name, baudrate=56700)
        self.log = logging.getLogger("CameraMock")

        self._resolution = PhotoResolution.p128

        self._on_ack = None

        self._command_handlers = {
            CommandCode.Reset: self._reset,
            CommandCode.Sync: self._sync,
            CommandCode.ACK: self._ack,
            CommandCode.Initial: self._initial,
            CommandCode.GetPicture: self._get_picture,
            CommandCode.SetPackageSize: self._set_package_size,
            CommandCode.Snapshot: self._snapshot
        }

        self.on_get_photo = None

    def handle(self):
        while True:
            cmd = self._port.read(6)

            command_id = ord(cmd[1])

            cmd_str = ensure_byte_list(cmd)

            self.log.debug('Got command %d (%s)', command_id, str(cmd_str))

            handler = self._command_handlers[command_id]
            handler(*cmd_str[2:])

    def _command(self, code, *payload):
        raw = [0xAA, code] + list(payload) + (4 - len(payload)) * [0]
        self._port.write(raw)

    def _reset(self, *cmd):
        self.log.debug('Reset (%s)', str(cmd))
        self._on_ack = None

    def _sync(self, *cmd):
        self._on_ack = None
        self.log.debug('Sync (%s)', str(cmd))
        self._command(CommandCode.ACK, CommandCode.Sync)
        self._command(CommandCode.Sync)

    def _ack(self, *cmd):
        self.log.debug('Ack (%s)', str(cmd))
        if self._on_ack is not None:
            self._on_ack(*cmd)

    def _initial(self, ignore, image_format, raw_resolution, jpeg_resolution):
        self._resolution = PhotoResolution(jpeg_resolution)
        self.log.info('Initial (Format={}, RAW Resolution={}, JPEG Resolution={})'.format(image_format, raw_resolution, self._resolution))
        self._command(CommandCode.ACK, CommandCode.Initial)

    def _get_picture(self, picure_type, *rest):
        photo_buffer = call(self.on_get_photo, [0xCC] * 10, self._resolution)

        def send_package(command_id, counter, package_id_0, package_id_1):
            package_id = (package_id_1 << 8) | package_id_0
            self.log.info('SendPackage(Package ID={})'.format(package_id))

            if package_id == 0xF0F0:
                self._on_ack = None
                self.log.info('Sending packages finished')
                return

            part = photo_buffer[package_id * 506:package_id * 506 + 506]

            package = [package_id_0, package_id_1, 0xFA, 0x01] + part + [0, 0]

            a = self._port.write(package)
            self.log.info('Written {}'.format(a))

        self._on_ack = send_package

        self.log.info('Get Picture({})'.format(picure_type))
        self._command(CommandCode.ACK, CommandCode.GetPicture)

        raw = ensure_byte_list(struct.pack('<L', len(photo_buffer)))[0:3]
        self._command(CommandCode.Data, 0x01, *raw)

    def _set_package_size(self, ignore, low_byte, high_byte, ignore2):
        package_size = (high_byte << 8) | low_byte
        self.log.info('Set Package Size({})'.format(package_size))
        self._command(CommandCode.ACK, CommandCode.SetPackageSize)

    def _snapshot(self, picture_type, counter_low, counter_high, ignore):
        counter = (counter_high << 8) | counter_low
        self.log.info('Snapshot(Picture Type={}, Counter={})'.format(picture_type, counter))
        self._command(CommandCode.Snapshot, CommandCode.Snapshot)


parser = argparse.ArgumentParser(
    fromfile_prefix_chars='@',
    epilog="In file path you can use {camera} as camera name (wing/nadir) and {res} as resolution (128, 240, 480)")
parser.add_argument("--nadir-port", help="Nadir COM port", required=True)
parser.add_argument("--wing-port", help="Wing COM port", required=True)
parser.add_argument("--photo-path", help="Photo path", required=True)


def convert_arg_line_to_args(arg_line):
    for arg in arg_line.split():
        if not arg.strip():
            continue
        yield arg


parser.convert_arg_line_to_args = convert_arg_line_to_args

args = parser.parse_args()
print args


def read_file(p):
    with open(p, 'rb') as f:
        return list(f.read())

wing = CameraMock(args.wing_port)
wing.on_get_photo = lambda r: read_file(args.photo_path.format(camera='wing', res=str(r)[:-1]))

nadir = CameraMock(args.nadir_port)
nadir.on_get_photo = lambda r: read_file(args.photo_path.format(camera='nadir', res=str(r)[:-1]))

wing_thread = Thread(target=lambda: wing.handle())
nadir_thread = Thread(target=lambda: nadir.handle())

wing_thread.start()
nadir_thread.start()

time.sleep(-1)



