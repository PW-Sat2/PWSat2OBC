from .obc_mixin import OBCMixin, command, decode_return

class StateMixin(OBCMixin):
    def __init__(self):
        pass
    
    @command("state get antenna")
    def state_get_antenna(self):
        pass

    @command("state get time_state")
    def state_get_time_state(self):
        pass

    @command("state get time_config")
    def state_get_time_config(self):
        pass

    @command("state get adcs")
    def state_get_adcs_config(self):
        pass

    @command("state set antenna {0}")
    def state_set_antenna(self, deployment_disabled):
        pass

    @command("state set time_state {0} {1}")
    def state_set_time_state(self, mission_time, external_time):
        pass

    @command("state set time_config {0} {1}")
    def state_set_time_config(self, internal_factor, external_factor):
        pass

    @command("state set adcs {0}")
    def state_set_adcs_config(self, detumbling_disabled):
        pass
