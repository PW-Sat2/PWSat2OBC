from math import ceil
from unittest import skip

from system import auto_power_on
from telecommand import EraseBootTableEntry, WriteProgramPart, FinalizeProgramEntry
from tests.base import BaseTest
from utils import TestEvent
from crc import calc_crc, pad


class UploadProgramTest(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(UploadProgramTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on(clean_state=False)
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    # @skip('Manual test')
    def test_upload(self):
        self._start()

        # erase entry
        self.system.comm.put_frame(EraseBootTableEntry([2]))

        # wait for response
        f = self.system.comm.get_frame(20)
        print f

        # upload entries 0...n
        program_path = 'D:/PW-Sat/obc-build/build/FlightModel/bin/fm_terminal.bin'

        with open(program_path, 'rb') as f:
            content = f.read()

        content = pad(content, multiply_of=128, pad_with=0x1A)

        crc = calc_crc(content)
        length = len(content)
        parts = int(ceil(length / float(WriteProgramPart.MAX_PART_SIZE)))

        for i in xrange(0, length, WriteProgramPart.MAX_PART_SIZE):
            part = content[i:i+WriteProgramPart.MAX_PART_SIZE]
            self.system.comm.put_frame(WriteProgramPart(entries=[2], offset=i, content=part))

        print "Total parts {}\nlength: {}\nCRC: {:X}".format(parts, length, crc)

        f = [self.system.comm.get_frame(20) for _ in xrange(0, parts)]
        print f

        # finalize entry
        name = 'Uploaded'
        self.system.comm.put_frame(FinalizeProgramEntry([2], length, crc, name))

        f = self.system.comm.get_frame(20)
        print f

        # reboot to slot?
