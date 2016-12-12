from .obc_mixin import OBCMixin, command, decode_return

class MissionMixin(OBCMixin):
    def __init__(self):
        pass

    @command("suspend_mission")
    def suspend_mission(self):
         pass

    @command("resume_mission")
    def resume_mission(self):
         pass

    @command("run_mission")
    def run_mission(self):
         pass
