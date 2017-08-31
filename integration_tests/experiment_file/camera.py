from parsec import count

from base import pid, label_as
from parsing import byte

CameraSyncCount = pid(0x37) >> byte
CameraSyncCount >>= label_as('Camera Sync Count')
