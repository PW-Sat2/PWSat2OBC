from telecommand.base import Telecommand, CorrelatedTelecommand


class SetAntennaDeployment(CorrelatedTelecommand):
    def __init__(self, correlation_id, deployment_disabled):
        super(SetAntennaDeployment, self).__init__(correlation_id)
        self._deployment_disabled = deployment_disabled

    def apid(self):
        return 0xA3

    def payload(self):
        return [self._correlation_id, int(self._deployment_disabled)]
