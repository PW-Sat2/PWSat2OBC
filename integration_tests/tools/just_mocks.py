import argparse
import imp
import logging
import os
import sys
from threading import Thread
from time import sleep
import colorlog
from datetime import datetime
import binascii
from SocketServer import ThreadingTCPServer, BaseRequestHandler

try:
    from i2cMock import I2CMock
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from i2cMock import I2CMock


def _setup_log():
    root_logger = logging.getLogger()

    handler = colorlog.StreamHandler()

    formatter = colorlog.ColoredFormatter(
        "%(log_color)s%(asctime)-15s %(levelname)s: [%(name)s] %(message)s",
        log_colors={
            'DEBUG': 'cyan',
            'INFO': 'green',
            'WARNING': 'yellow',
            'ERROR': 'red',
            'CRITICAL': 'red,bg_white',
        }
    )

    handler.setFormatter(formatter)

    root_logger.addHandler(handler)
    root_logger.setLevel(logging.DEBUG)


AVAILABLE_DEVICES = {
    'eps.a': lambda c: c.eps.controller_a,
    'eps.b': lambda c: c.eps.controller_b,
    'transmitter': lambda c: c.transmitter,
    'receiver': lambda c: c.receiver,
    'antenna.primary': lambda c: c.primary_antenna,
    'antenna.backup': lambda c: c.backup_antenna,
    'imtq': lambda c: c.imtq,
    'rtc': lambda c: c.rtc,
    'gyro': lambda c: c.gyro,
}


class AllButSelected(argparse.Action):
    def __init__(self, **kwargs):
        self._choices = kwargs['choices']
        super(AllButSelected, self).__init__(**kwargs)

    def __call__(self, parser, namespace, values, option_string=None):
        result = list(self._choices)
        for v in values:
            result.remove(v)
        setattr(namespace, self.dest, result)


def convert_arg_line_to_args(arg_line):
    for arg in arg_line.split():
        if not arg.strip():
            continue
        yield arg


def parse_args():
    parser = argparse.ArgumentParser(
        fromfile_prefix_chars='@',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Starts subset of available mock devices",
        epilog="""
Availble devices: \n\t{devices}
Arguments may also be stored in file and passed with '@file' syntax. Arguments in file should be separated with whitespace
        """.format(
            devices='\n\t'.join(sorted(AVAILABLE_DEVICES.keys()))
        )
    )
    parser.convert_arg_line_to_args = convert_arg_line_to_args

    parser.add_argument('-c', '--config', required=True,
                        help="Config file (in CMake-generated integration tests format, only MOCK_COM required)",)

    parser.add_argument('-f', '--frames', help="Save all transmitted frames to file", default=None)

    parser.add_argument("--comm-tcp", help="Start COMM TCP server on given port", dest='comm_tcp', type=int)

    parser.add_argument('-d', '--debug', action='store_true', default=False, help="Enable I2C logs")

    device_selection_group = parser.add_mutually_exclusive_group(required=True)
    device_selection_group.add_argument('--except',
                                        help="Enable all devices except selected",
                                        dest='enabled_devices',
                                        action=AllButSelected,
                                        choices=AVAILABLE_DEVICES.keys(),
                                        nargs='+',
                                        metavar='DEVICE'
                                        )
    device_selection_group.add_argument('--only',
                                        help="Enable only selected devices",
                                        dest='enabled_devices',
                                        choices=AVAILABLE_DEVICES.keys(),
                                        nargs='+',
                                        metavar='DEVICE'
                                        )
    device_selection_group.add_argument('--all',
                                        help="Enable all devices",
                                        dest='enabled_devices',
                                        action='store_const',
                                        const=AVAILABLE_DEVICES.keys()
                                        )

    return parser.parse_args()


def read_all(s, size):
    result = ""
    while size > 0:
        part = s.recv(size)
        result += part
        size -= len(part)

    return result


class CommHandler(BaseRequestHandler):
    log = logging.getLogger("Comm.TCP")

    def _send_frame(self):
        size = ord(self.request.recv(1))
        frame = read_all(self.request, size)

        self.server.comm.receiver.put_frame(frame)

        self.request.sendall('ACK')

    def _receive_frame(self):
        self.request.sendall('ACK')

    def handle(self):
        command = self.request.recv(1)

        if command == 'S':
            self._send_frame()
        elif command == 'R':
            self._receive_frame()
        else:
            self.request.sendall('NAK')


class JustMocks(object):
    def __init__(self, mock_com):
        self._mock_com = mock_com

        self.i2c = I2CMock(mock_com)

        import response_frames
        from devices import EPS, Comm, AntennaController, Imtq, Gyro, RTCDevice
        from devices import BACKUP_ANTENNA_CONTROLLER_ADDRESS, PRIMARY_ANTENNA_CONTROLLER_ADDRESS

        self.frame_decoder = response_frames.FrameDecoder(response_frames.frame_factories)

        self.eps = EPS()
        self.comm = Comm(self.frame_decoder)
        self.transmitter = self.comm.transmitter
        self.receiver = self.comm.receiver
        self.primary_antenna = AntennaController(PRIMARY_ANTENNA_CONTROLLER_ADDRESS, "Primary Antenna")
        self.backup_antenna = AntennaController(BACKUP_ANTENNA_CONTROLLER_ADDRESS, "Backup Antenna")
        self.imtq = Imtq()
        self.gyro = Gyro()
        self.rtc = RTCDevice()

        self.i2c.add_bus_device(self.eps.controller_a)
        self.i2c.add_pld_device(self.eps.controller_b)
        self.i2c.add_bus_device(self.transmitter)
        self.i2c.add_bus_device(self.receiver)
        self.i2c.add_bus_device(self.primary_antenna)
        self.i2c.add_pld_device(self.backup_antenna)
        self.i2c.add_bus_device(self.imtq)
        self.i2c.add_pld_device(self.rtc)
        self.i2c.add_pld_device(self.gyro)

    def start(self, devices_to_enable):
        self.i2c.start(enable_devices=False)
        addresses = map(lambda d: d(self).address, devices_to_enable.values())
        self.i2c.enable_devices(addresses, True)

    def stop(self):
        self.i2c.stop()

    def save_transmitted_frames_to(self, file_path):
        def log_frame(_, content):
            stamp = datetime.now().isoformat()
            frame_data = binascii.hexlify(bytearray(content)).upper()

            with open(file_path, 'a') as f:
                f.write('{}|{}\n'.format(stamp, frame_data))

            print 'Logging frame {}'.format(len(content))
        self.comm.transmitter.on_send_frame = log_frame

    def start_comm_server(self, port):
        print 'Starting COMM TCP server'
        server = ThreadingTCPServer(('0.0.0.0', port), CommHandler)
        server.comm = self.comm
        f = lambda: server.serve_forever()
        t = Thread(target=f)
        t.daemon = True
        t.start()

_setup_log()

args = parse_args()

logging.getLogger("I2C").propagate = True

if not args.debug:
    logging.getLogger("I2C").setLevel(logging.WARN)

config = imp.load_source('config', args.config)

devices_to_enable = {k: AVAILABLE_DEVICES[k] for k in args.enabled_devices}

just_mocks = JustMocks(mock_com=config.config['MOCK_COM'])

print 'Starting devices: {}'.format(', '.join(sorted(devices_to_enable.keys())))

if args.frames is not None:
    just_mocks.save_transmitted_frames_to(args.frames)

just_mocks.start(devices_to_enable)

print '{} | {}'.format(type(args.comm_tcp), args.comm_tcp)
if args.comm_tcp is not None:
    just_mocks.start_comm_server(args.comm_tcp)

print 'Press Ctrl+C to stop'
try:
    sleep(-1)
except KeyboardInterrupt:
    print 'Stopping'
    just_mocks.stop()
