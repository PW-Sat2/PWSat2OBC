from telecommand import Telecommand


class SetAntennaDeployment(Telecommand):
    def __init__(self, correlation_id, deployment_disabled):
        Telecommand.__init__(self)
        self._correlation_id = correlation_id
        self._deployment_disabled = deployment_disabled

    def apid(self):
        return 0xA3

    def payload(self):
        return [self._correlation_id, int(self._deployment_disabled)]
