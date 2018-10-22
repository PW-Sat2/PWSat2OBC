from enum import unique, IntEnum


@unique
class ExperimentType(IntEnum):
    Detumbling = 1
    EraseFlash = 2
    LEOP = 4
    SunS = 3
    RadFET = 5
    SADS = 6
    Sail = 7
    Fibo = 8
    Payload = 9
    Camera = 10


@unique
class StartResult(IntEnum):
    Success = 0
    Failure = 1


@unique
class IterationResult(IntEnum):
    Finished = 0
    LoopImmediately = 1
    WaitForNextCycle = 2,
    Failure = 3
