from base64 import b64encode
from string import Formatter


class CommandFormatter(Formatter):
    def convert_field(self, value, conversion):
        if conversion == 'E':
            return b64encode(value).rstrip('=')

        return super(CommandFormatter, self).convert_field(value, conversion)