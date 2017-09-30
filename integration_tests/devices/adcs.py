from enum import IntEnum, unique

@unique
class AdcsMode(IntEnum):
    Disabled = -2
    Stopped = -1
    BuiltinDetumbling = 0
    ExperimentalDetumbling = 1
    ExperimentalSunpointing = 2
