from .obc_mixin import OBCMixin, command, decode_multiline_dictionary


class MCUTemperatureMixin(OBCMixin):
    def __init__(self):
        pass

    @decode_multiline_dictionary()
    @command("temp")
    def mcu_temp(self):
        pass

    @decode_multiline_dictionary()
    @command("temp raw")
    def mcu_temp_raw(self):
        pass

    @decode_multiline_dictionary()
    @command("temp calibration")
    def mcu_temp_calibration(self):
        pass
