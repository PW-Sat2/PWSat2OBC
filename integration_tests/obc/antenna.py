from .obc_mixin import OBCMixin, command, decode_return

def AntennaMixin(OBCMixin):
    def __init__(self):
        pass

    @command("antenna arm")
    def antenna_arm_deployment(self):
        pass;

    @command("antenna disarm")
    def antenna_disarm_deployment(self):
        pass;

    @command("antenna deploy {0}")
    def antenna_deploy(self, antennaId):
        pass;

    @command("antenna deploy auto")
    def antenna_autodeploy(self):
        pass;

    @command("antenna deploy cancel")
    def antenna_cancel_deployment(self):
        pass;
