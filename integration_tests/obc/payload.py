from .obc_mixin import OBCMixin, command, decode_multiline_dictionary


class PayloadMixin(OBCMixin):
    def __init__(self):
        pass

    @decode_multiline_dictionary()
    @command("payload temps")
    def payload_temps(self):
        pass

    @decode_multiline_dictionary()
    @command("payload suns")
    def payload_suns(self):
        pass

    @decode_multiline_dictionary()
    @command("payload who")
    def payload_whoami(self):
        pass

    @decode_multiline_dictionary()
    @command("payload photo")
    def payload_photodiodes(self):
        pass

    @decode_multiline_dictionary()
    @command("payload house")
    def payload_housekeeping(self):
        pass

    @decode_multiline_dictionary()
    @command("payload radfet on")
    def payload_radfet_on(self):
        pass

    @decode_multiline_dictionary()
    @command("payload radfet read")
    def payload_radfet_read(self):
        pass

    @decode_multiline_dictionary()
    @command("payload radfet off")
    def payload_radfet_off(self):
        pass