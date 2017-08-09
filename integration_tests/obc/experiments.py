import time
from enum import unique, IntEnum

from utils import busy_wait
from .obc_mixin import OBCMixin, command, decode_return


@unique
class ExperimentType(IntEnum):
    LEOP = 0
    Detumbling = 1
    EraseFlash = 2
    SunS = 3
    Sail = 7
    Fibo = 8


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


class ExperimentState:
    def __init__(self):
        self.Requested = None
        self.Current = None
        self.LastStartResult = None
        self.LastIterationResult = None
        self.IterationCounter = None

    def __str__(self):
        return '''
Requested: {0!s}
Current: {1!s}
LastStartResult: {2!s}
LastIterationResult: {3!s}
IterationCounter: {4}
'''.format(self.Requested, self.Current, self.LastStartResult, self.LastIterationResult, self.IterationCounter)


class ExperimentsMixin(OBCMixin):
    @command("request_experiment {0}")
    def request_experiment(self):
        pass

    @command("set_fibo_iterations {0}")
    def set_fibo_iterations(self, count):
        pass

    @command("abort_experiment")
    def abort_experiment(self):
        pass

    def _parse_experiment_state(text):
        lines = text.strip('\n').split('\n')
        v = dict(map(lambda x: x.split('\t'), lines))

        def decode(key, type):
            if v[key] == 'None':
                return None
            else:
                return type(int(v[key]))

        state = ExperimentState()
        state.Requested = decode('Requested', ExperimentType)
        state.Current = decode('Current', ExperimentType)
        state.LastStartResult = decode('LastStartResult', StartResult)
        state.LastIterationResult = decode('LastIterationResult', IterationResult)
        state.IterationCounter = int(v['IterationCounter'])

        return state

    @decode_return(_parse_experiment_state)
    @command("experiment_info")
    def experiment_info(self):
        pass

    def wait_for_experiment(self, experiment, timeout):
        def condition():
            info = self.experiment_info()
            return info.Current == experiment

        busy_wait(condition, delay=1, timeout=timeout)

    def wait_for_experiment_started(self, experiment, timeout):
        def condition():
            info = self.experiment_info()
            return info.Current == experiment and info.LastStartResult is not None

        busy_wait(condition, delay=1, timeout=timeout)

    def wait_for_experiment_iteration(self, iteration, timeout):
        def condition():
            info = self.experiment_info()
            return info.IterationCounter == iteration

        busy_wait(condition, delay=0.1, timeout=timeout)

