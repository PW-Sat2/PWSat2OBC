from .obc_mixin import OBCMixin, command, decode_return


class PayloadMixin(OBCMixin):
    def __init__(self):
        pass

    @command("payload radfet")
    def payload_radfet(self):
        pass

    @command("payload temps")
    def payload_temps(self):
        pass

    @command("payload suns")
    def payload_suns(self):
        pass

    @command("payload who")
    def payload_whoami(self):
        pass

    @command("payload photo")
    def payload_photodiodes(self):
        pass

    @command("payload house")
    def payload_housekeeping(self):
        pass