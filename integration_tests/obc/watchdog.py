from .obc_mixin import OBCMixin, command


class WatchdogMixin(OBCMixin):
    @command("hang 42")
    def hang(self):
        pass
