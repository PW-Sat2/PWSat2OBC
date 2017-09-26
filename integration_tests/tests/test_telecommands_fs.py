import struct

import telecommand
from response_frames.common import FileRemoveErrorFrame, FileSendErrorFrame
from response_frames.common import DownlinkApid
from system import auto_power_on, runlevel
from tests.base import RestartPerTest
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

        p = "/test"

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

        p = "/non_exist"

        self.system.comm.put_frame(telecommand.DownloadFile(correlation_id=0x11, path=p, seqs=[0, 3, 1, 2]))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, FileSendErrorFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.error_code, 1)

    @runlevel(2)
    def test_should_remove_file(self):
        self._start()

        data = "content"

        p = "/test"

        self.system.obc.write_file(p, data)

        self.system.comm.put_frame(telecommand.RemoveFile(correlation_id=0x11, path=p))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), DownlinkApid.FileRemove)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.response, ensure_byte_list(p))

    @runlevel(2)
    def test_should_report_error_when_removing_non_existent_file(self):
        self._start()

        p = "/non_exist"

        self.system.comm.put_frame(telecommand.RemoveFile(correlation_id=0x11, path=p))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, FileRemoveErrorFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.error_code, 0xFE)
        self.assertEqual(frame.response, ensure_byte_list(p))
        self.assertEqual(frame.payload(), [0x11, 0xFE] + ensure_byte_list(p))

    @runlevel(2)
    def test_should_list_files(self):
        self._start()

        self.system.obc.write_file('/file1', 'ABC')
        self.system.obc.write_file('/file2', 'DEFG')
        self.system.obc.write_file('/file3', 'HI')

        self.system.comm.put_frame(telecommand.ListFiles(correlation_id=0x11, path='/'))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), DownlinkApid.FileList)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.payload()[0], 0x11)
        self.assertEqual(frame.payload()[1], 0)

        rest = frame.payload()[2:]
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

