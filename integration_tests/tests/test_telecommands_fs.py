import struct

from nose.tools import nottest

import telecommand
from response_frames.operation import OperationErrorFrame
from system import auto_power_on, runlevel
from tests.base import BaseTest, RestartPerTest
from utils import ensure_byte_list, TestEvent


class FileSystemTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(FileSystemTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @runlevel(2)
    def test_receive_multipart_file(self):
        self._start()

        data = ''.join(map(lambda x: x * 300, ['A', 'B', 'C']))

        p = "/a/test"

        self.system.obc.write_file(p, data)

        self.system.comm.put_frame(telecommand.DownloadFile(correlation_id=0x11, path=p, seqs=[0, 3, 1, 2]))

        frames = [
            self.system.comm.get_frame(20),
            self.system.comm.get_frame(1),
            self.system.comm.get_frame(1),
            self.system.comm.get_frame(1)
        ]

        frames = sorted(frames, key=lambda x: x.seq())

        received = ''
        for f in frames:
            received += ''.join([chr(b) for b in f.payload()[2:]])

        self.assertAlmostEqual(received, data)

    @runlevel(2)
    def test_should_respond_with_error_frame_for_non_existent_file_when_downloading(self):
        self._start()

        p = "/a/non_exist"

        self.system.comm.put_frame(telecommand.DownloadFile(correlation_id=0x11, path=p, seqs=[0, 3, 1, 2]))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationErrorFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.error_code, 1)

    @runlevel(2)
    def test_should_remove_file(self):
        self._start()

        data = "content"

        p = "/a/test"

        self.system.obc.write_file(p, data)

        self.system.comm.put_frame(telecommand.RemoveFile(correlation_id=0x11, path=p))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), 2)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.response, ensure_byte_list(p))

    @runlevel(2)
    def test_should_report_error_when_removing_non_existent_file(self):
        self._start()

        p = "/a/non_exist"

        self.system.comm.put_frame(telecommand.RemoveFile(correlation_id=0x11, path=p))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationErrorFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.error_code, 0xFE)
        self.assertEqual(frame.response, ensure_byte_list(p))
        self.assertEqual(frame.payload(), [0x11, 0xFE] + ensure_byte_list(p))

    @runlevel(2)
    def test_should_list_files(self):
        self._start()

        self.system.obc.write_file('/a/file1', 'ABC')
        self.system.obc.write_file('/a/file2', 'DEFG')
        self.system.obc.write_file('/a/file3', 'HI')

        self.system.comm.put_frame(telecommand.ListFiles(correlation_id=0x11, path='/a'))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), 2)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.payload()[0], 0x11)

        rest = frame.payload()[1:]
        files = []
        while len(rest) > 0:
            l = rest.index(0)
            p = ''.join(map(chr, rest[0:l]))
            (size,) = struct.unpack('<L', ''.join(map(chr, rest[l + 1: l + 5])))

            rest = rest[l+5:]
            files.append((p, size))

        self.assertIn(('file1', 3), files)
        self.assertIn(('file2', 4), files)
        self.assertIn(('file3', 2), files)

