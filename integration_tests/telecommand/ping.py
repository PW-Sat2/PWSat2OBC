from telecommand import Telecommand

class PingTelecommand(Telecommand):
    def __init__(self):
        Telecommand.__init__(self)

    def apid(self):
        return 0x50

    def payload(self):
        return []