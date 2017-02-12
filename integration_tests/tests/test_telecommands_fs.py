import struct

from devices import UplinkFrame
from tests.base import BaseTest
from utils import ensure_byte_list


class FileSystemTelecommandsTest(BaseTest):
    def test_receive_multipart_file(self):
        data = ''.join(map(lambda x: x * 300, ['A', 'B', 'C']))

        p = "/a/test"

        self.system.obc.write_file(p, data)

        seqs = ensure_byte_list(struct.pack('<LLLL', 0, 3, 1, 2))

        self.system.comm.put_frame(UplinkFrame(apid=0xAB, content=[0x11, len(p)] + list(p) + [0x0] + seqs))

        frames = [
            self.system.comm.get_frame(20),
            self.system.comm.get_frame(20),
            self.system.comm.get_frame(20),
            self.system.comm.get_frame(20)
        ]

        frames = sorted(frames, key=lambda x: x.seq())

        received = ''
        for f in frames:
            received += ''.join([chr(b) for b in f.payload()])

        self.assertAlmostEqual(received, data)