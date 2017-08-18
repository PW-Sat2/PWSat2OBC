from base import *
from enum import Enum, unique

@unique
class SailOpeningState(Enum):
    Waiting = 0
    Opening = 1
    Open = 2

sailState = packed('<B').parsecmap(lambda x: SailOpeningState(x))

SailState = joint(
        field('Deployed', sailState)
    ).bind(to_dict)
SailState >>= label_as('Sail State')
