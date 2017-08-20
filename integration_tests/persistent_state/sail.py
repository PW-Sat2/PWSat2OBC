from parsec import joint
from parsing import *
from enum import Enum, unique

@unique
class SailOpeningState(Enum):
    Waiting = 0
    Opening = 1
    Open = 2

sailState = packed('<B').parsecmap(lambda x: SailOpeningState(x))

SailState = field('Deployed', sailState)
SailState >>= label_as('Sail State')
