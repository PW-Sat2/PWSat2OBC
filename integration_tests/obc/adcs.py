from obc_mixin import OBCMixin, command, decode_return
from devices import AdcsMode

class ADCSMixin(OBCMixin):

    def _parse_adcs_mode(result):
        return AdcsMode(int(result.split(': ')[1]))

    @decode_return(_parse_adcs_mode)
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

    @command("adcs stop")
    def adcs_stop(self):
        pass
