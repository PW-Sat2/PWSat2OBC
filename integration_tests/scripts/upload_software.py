# run with %run -i scripts/upload_software.py file slot_1 slot_2 slot_3 description
import sys
import progressbar
from Queue import Empty

from math import ceil
from time import time

from crc import pad, calc_crc
from response_frames.program_upload import EntryEraseSuccessFrame, EntryProgramPartWriteSuccess, EntryFinalizeSuccess
from telecommand import WriteProgramPart, EraseBootTableEntry, FinalizeProgramEntry

PARTS_PER_ITERATION = 25


def wait_for_frame(expected_type, timeout):
    start_time = time()
    timeout_at = start_time + timeout

    while time() < timeout_at:
        try:
            frame = system.comm.get_frame(1)

            if type(frame) is expected_type:
                return frame

            print 'Ignoring {}'.format(frame)
        except Empty:
            pass

    return None


file = sys.argv[1]
slots = [int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])]
description = sys.argv[5]

with open(file, 'rb') as f:
    program_data = f.read()

program_data = pad(program_data, multiply_of=128, pad_with=0x1A)
crc = calc_crc(program_data)
length = len(program_data)
parts = int(ceil(length / float(WriteProgramPart.MAX_PART_SIZE)))

print 'Will upload {} bytes of program (CRC: {:4X}), {} parts into slots {}'.format(
    length,
    crc,
    parts,
    slots
)

print 'Erasing boot slots'
system.comm.put_frame(EraseBootTableEntry(slots))

response = wait_for_frame(EntryEraseSuccessFrame, 40)
if response is None:
    print 'Failed to erase'
    sys.exit(1)
print 'Boot slots erased'


print 'Uploading program'
offsets = range(0, length, WriteProgramPart.MAX_PART_SIZE)
total_parts = len(offsets)

with progressbar.ProgressBar(max_value=total_parts, redirect_stdout=True) as bar:
    bar.update(0)
    while len(offsets) > 0:
        parts = offsets[0:PARTS_PER_ITERATION]

        for offset in parts:
            part = program_data[offset:offset + WriteProgramPart.MAX_PART_SIZE]

            system.comm.put_frame(WriteProgramPart(entries=slots, offset=offset, content=part))

        for _ in parts:
            response = wait_for_frame(EntryProgramPartWriteSuccess, 120) # type: EntryProgramPartWriteSuccess

            if response is None:
                print 'Failed to program'
                sys.exit(2)

            if response.offset not in parts:
                print 'Invalid offset received {}'.format(response.offset)
                sys.exit(4)

            offsets.remove(response.offset)
            bar.update(total_parts - len(offsets))
    #
    # for i in xrange(0, len(offsets)):
    #     offset = offsets[i]
    #     print 'Uploading to offset 0x{:X} ({}/{})'.format(offset, i + 1, len(offsets))
    #
    #     part = program_data[offset:offset + WriteProgramPart.MAX_PART_SIZE]
    #
    #     system.comm.put_frame(WriteProgramPart(entries=slots, offset=offset, content=part))
    #
    #     response = wait_for_frame(EntryProgramPartWriteSuccess, 120)
    #
    #     if response is None:
    #         print 'Failed to program'
    #         sys.exit(2)
    #

print 'Upload finished'

print 'Finalizing'
system.comm.put_frame(FinalizeProgramEntry(slots, length, crc, description))

response = wait_for_frame(EntryFinalizeSuccess, 40)

if response is None:
    print 'Failed to finalize'
    sys.exit(3)

print 'Uploaded {} bytes of program (CRC: {:4X}), {} parts into slots {}'.format(
    length,
    crc,
    parts,
    slots
)