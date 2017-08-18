from telecommand import Telecommand


class GetPersistentState(Telecommand):
    def apid(self):
        return 0x21

    def payload(self):
        return []