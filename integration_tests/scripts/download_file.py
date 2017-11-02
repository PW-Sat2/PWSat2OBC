import sys
from Queue import Empty

from math import ceil

from response_frames.common import FileSendSuccessFrame
from telecommand import DownloadFile
from utils import ensure_string

obc_path = sys.argv[1]
length = int(sys.argv[2])
local_file = sys.argv[3]

chunks_count = int(ceil(length / 230.0))

print 'Downloading file from {} ({} bytes, {} chunks)'.format(obc_path, length, chunks_count)

chunks = range(0, chunks_count)

i = 0

CHUNKS_PER_ITERATION = 10

with open(local_file, 'wb') as local:
    while i < chunks_count:
        r = (i, min(i + CHUNKS_PER_ITERATION, chunks_count))
        i += CHUNKS_PER_ITERATION

        chunks = range(*r)

        print 'Requesting...'
        system.comm.put_frame(DownloadFile(0x45, obc_path, chunks))

        while len(chunks) > 0:
            print '\tWaiting for chunks {}:'.format(chunks)
            try:
                part = system.comm.get_frame(1) # type: OperationSuccessFrame
            except Empty:
                print '\t\tTimeout waiting for frame'

            if type(part) is not FileSendSuccessFrame:
                print '\t\tIgnoring {} (not success)'.format(part)
                continue

            if part.correlation_id != 0x45:
                print '\t\tIgnoring {} (not matching correlation id)'.format(part)
                continue

            if part.seq() not in chunks:
                print '\t\tIgnoring {} (not matching SEQ)'.format(part)
                continue

            print '\t\tReceived chunk {}'.format(part.seq())
            chunks.remove(part.seq())

            local.seek(part.seq() * 230, 0)
            local.write(ensure_string(part.response))




