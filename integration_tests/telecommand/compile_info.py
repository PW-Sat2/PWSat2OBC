from telecommand import Telecommand


class GetCompileInfoTelecommand(Telecommand):
    def __init__(self):
        Telecommand.__init__(self)

    def apid(self):
        return 0x27

    def payload(self):
        return []