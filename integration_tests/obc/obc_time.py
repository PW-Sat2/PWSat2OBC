import datetime

from .obc_mixin import OBCMixin, command, decode_return, decode_from_miliseconds


class TimeMixin(OBCMixin):
    def __init__(self):
        pass

    @command("jumpToTime {0!t}")
    def jump_to_time(self, time):
        # type: (datetime.timedelta) -> None
        pass

    @command("advance_time {0!t}")
    def advance_time(self, delta):
        pass

    @decode_from_miliseconds()
    @command("currentTime")
    def current_time(self):
        # type: () -> datetime.timedelta
        pass

    @command("rtc get")
    def rtc_info(self):
        pass

    @decode_return(int)
    @command("rtc duration")
    def rtc_duration(self):
        pass

    @command("rtc test")
    def rtc_test(self):
        pass
