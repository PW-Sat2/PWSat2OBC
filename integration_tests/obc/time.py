from .obc_mixin import OBCMixin, command, decode_return


class TimeMixin(OBCMixin):
    def __init__(self):
        pass

    @command("jumpToTime {0}")
    def jump_to_time(self, time):
        pass

    @decode_return(int)
    @command("currentTime")
    def current_time(self):
        pass
