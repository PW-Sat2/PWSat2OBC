from .obc_mixin import OBCMixin, command


class RunlevelMixin(OBCMixin):
    @command("runlevel start comm")
    def runlevel_start_comm(self):
        pass