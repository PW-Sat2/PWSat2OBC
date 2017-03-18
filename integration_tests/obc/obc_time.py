from .obc_mixin import OBCMixin, command, decode_return


class TimeMixin(OBCMixin):
    def __init__(self):
        pass

    @command("jumpToTime {0}")
    def jump_to_time(self, time):
        pass

    @command("advance_time {0}")
    def advance_time(self, delta):
        pass

    @decode_return(int)
    @command("currentTime")
    def current_time(self):
        pass

    @command("rtc get")
    def rtc_info(self):
        pass

    @decode_return(int)
    @command("rtc duration")
    def rtc_duration(self):
        pass
