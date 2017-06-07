from .obc_mixin import OBCMixin, command, decode_return


def _parse_boot_settings(s):
    lines = s.split('\n')
    m = dict(map(lambda l: l.split(':'), lines))

    return {
        'Primary': int(m['Primary boot slots'].strip(), 16),
        'Failsafe': int(m['Failsafe boot slots'].strip(), 16),
        'Counter': int(m['Boot counter'].strip()),
        'Confirmed': m['Boot confirmed'].strip() == "Yes"
    }


class BootSettingsMixin(OBCMixin):
    @decode_return(_parse_boot_settings)
    @command("boot_settings get")
    def boot_settings(self):
        pass