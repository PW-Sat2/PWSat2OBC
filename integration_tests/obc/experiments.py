from enum import unique, IntEnum

from .obc_mixin import OBCMixin, command


@unique
class ExperimentType(IntEnum):
    Fibo = 1


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
