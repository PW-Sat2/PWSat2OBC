from enum import unique, Enum

from .obc_mixin import OBCMixin, command


@unique
class ExperimentType(Enum):
    Fibo = 1


class ExperimentsMixin(OBCMixin):
    @command("request_experiment {0}")
    def request_experiment(self):
        pass
