from parsec import count

from base import pid, label_as
from parsing import byte

CameraSyncCount = pid(0x37) >> count(byte, 1)
CameraSyncCount >>= label_as('Camera Sync Count')
