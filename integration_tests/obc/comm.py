from .obc_mixin import OBCMixin, command, decode_return


class CommMixin(OBCMixin):
    def __init__(self):
        pass

    @command("sendFrame {0}")
    def send_frame(self, data):
        pass

    @decode_return(int)
    @command("getFramesCount")
    def get_frame_count(self):
        pass

    @command("receiveFrame")
    def receive_frame(self):
        pass

    def comm_auto_handling(self, enable):
        if not enable:
            self._command("pauseComm")
