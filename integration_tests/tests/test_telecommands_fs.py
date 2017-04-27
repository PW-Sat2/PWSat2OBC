from nose.tools import nottest

import telecommand
from system import auto_power_on
from tests.base import BaseTest
from utils import ensure_byte_list, TestEvent


class FileSystemTelecommandsTest(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(FileSystemTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on(clean_state=True)
        self.system.obc.wait_to_start()

        e.wait_for_change(1)
    @nottest
    def test_receive_multipart_file(self):
        self._start()

        data = ''.join(map(lambda x: x * 300, ['A', 'B', 'C']))

        p = "/a/test"

        self.system.obc.write_file(p, data)

        self.system.comm.put_frame(telecommand.DownloadFile(respond_as=0x11, path=p, seqs=[0, 3, 1, 2]))

        frames = [
            self.system.comm.get_frame(20),
            self.system.comm.get_frame(1),
            self.system.comm.get_frame(1),
            self.system.comm.get_frame(1)
        ]

        frames = sorted(frames, key=lambda x: x.seq())

        received = ''
        for f in frames:
            received += ''.join([chr(b) for b in f.payload()])

        self.assertAlmostEqual(received, data)

    def test_should_respond_with_error_frame_for_non_existent_file(self):
        self._start()

        p = "/a/non_exist"

        self.system.comm.put_frame(telecommand.DownloadFile(respond_as=0x11, path=p, seqs=[0, 3, 1, 2]))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), 2)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.payload(), ensure_byte_list(p))
