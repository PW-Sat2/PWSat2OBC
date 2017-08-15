from obc_mixin import OBCMixin, command


class ADCSMixin(OBCMixin):
    @command("adcs current")
    def adcs_current(self):
        pass

    @command("adcs disable")
    def adcs_disable(self):
        pass

    @command("adcs builtin")
    def adcs_enable_builtin(self):
        pass

    @command("adcs exp_dtb")
    def adcs_enable_experimental_detumbling(self):
        pass
