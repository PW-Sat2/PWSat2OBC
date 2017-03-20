import logging
import i2cMock
from utils import *

PRIMARY_ANTENNA_CONTROLLER_ADDRESS = 0x31
BACKUP_ANTENNA_CONTROLLER_ADDRESS = 0x32

class Antenna(object):
    def __init__(self):
        self.deployed = False
        self.activation_count = 0
        self.activation_time = 0
        self.is_being_deployed = False

    @staticmethod
    def build(deployed, activation_count, activation_time, deployInProgress):
        value = Antenna()
        value.deployed = deployed
        value.activation_count = activation_count
        value.activation_time = activation_time
        value.is_being_deployed = deployInProgress
        return value

    def deployment_in_progress(self):
        return self.is_being_deployed

    def is_deployed(self):
        return self.deployed

    def begin_deployment(self):
        self.activation_count += 1
        self.is_being_deployed = True

    def end_deployment(self):
        self.deployed = True
        self.is_being_deployed = False

    def cancel_deployment(self):
        self.is_being_deployed = False

class AntennaController(i2cMock.I2CDevice):
    def __init__(self, address):
        super(AntennaController, self).__init__(address)
        self.log = logging.getLogger("Antenna" + str(address))
        self.armed = False
        self.ignore_deployment_switch = False
        self.antenna_state = [Antenna(), Antenna(), Antenna(), Antenna()]
        self.deployment_in_progress = False

        # callback called when controller is being reset
        # expected prototype:
        # None -> None
        self.on_reset = None

        # callback called when controller arming state is being changed
        # expected prototype:
        # bool requestedState -> bool|None 
        # parameters: 
        # requestedState Flag indicating requested system state, True for armed, false otherwise
        # This callback can return new state that will be set as command result or it can
        # omit the return value in this case the default behaviour will be applied.
        self.on_arm_state_change = None

        # callback called when the antenna deployment is being started
        # expected prototype:
        # AntennaController controller, int antennaId -> bool|None
        # parameters:
        # controller reference to the controller object that is being affected
        # antennaId Identifier of the antenna whose deployment process is being deployed. 
        #   Antenna ids have positive ids, value -1 indicates automatic deployment.
        # this callback can return bool indicating new deployment process state. 
        # True for the antenna being currently deployed, false in case the deployment process
        # should not start. Returning None will indicate that default behaviour should be used.
        self.on_begin_deployment = None

        # callback called when controller temperature is being requested.
        # expected prototype:
        # None -> int|None
        # parameters:
        # None
        # this callback can return an integer that should be used as current temperature 
        # measurement. Returning None will indicate that default value should be used.
        self.on_get_temperature = None

        # callback called when controller's deployment status is being requested.
        # expected prototype:
        # None -> []|None
        # parameters: 
        # None
        # this callback can return the array of bytes that should be returned directly as query result.
        # returning None will indicate that default behaviour should be used.
        self.on_get_deployment_status = None

        # callback called when deployment process should be aborted. 
        # expected prototype:
        # None -> None
        self.on_deployment_cancel = None

    def reset_state(self):
        self.armed = False
        self.ignore_deployment_switch = False
        self.antenna_state = [Antenna(), Antenna(), Antenna(), Antenna()]
        self.deployment_in_progress = False

    # antenna icd section 6.2.1
    @i2cMock.command([0xAA])
    def reset(self):
        self.log.debug("Resetting antenna controller")
        if call(self.on_reset, True):
            self.reset_state()

    # antenna icd section 6.2.2
    @i2cMock.command([0xAD])
    def _arm_deployment_system(self):
        self.log.debug("Arming deployment system controller")
        if not self.armed:
            self.armed = call(self.on_arm_state_change, True, True)

    # antenna icd section 6.2.3
    @i2cMock.command([0xAC])
    def _disarm_deployment_system(self):
        self.log.debug("Disarming deployment system controller")
        if self.armed:
            self.armed = call(self.on_arm_state_change, False, False)

    def deploy_antenna(self, antanna_id, timeout):
        self.log.debug("Beginning of deployment antenna %d on controller", antanna_id)
        if call(self.on_begin_deployment, True, self, antanna_id):
            self.antenna_state[antanna_id - 1].begin_deployment()

    # antenna icd section 6.2.4
    @i2cMock.command([0xA1])
    def _deploy_antenna_1(self, timeout):
        self.deploy_antenna(1, timeout)

    # antenna icd section 6.2.5
    @i2cMock.command([0xA2])
    def _deploy_antenna_2(self, timeout):
        self.deploy_antenna(2, timeout)

    # antenna icd section 6.2.6
    @i2cMock.command([0xA3])
    def _deploy_antenna_3(self, timeout):
        self.deploy_antenna(3, timeout)

    # antenna icd section 6.2.7
    @i2cMock.command([0xA4])
    def _deploy_antenna_4(self, timeout):
        self.deploy_antenna(4, timeout)

    def deploy_antenna_with_override(self, antanna_id, timeout):
        self.ignore_deployment_switch = True
        self.deploy_antenna(antanna_id, timeout)

    # antenna icd section 6.2.9
    @i2cMock.command([0xBA])
    def _deploy_antenna_1_with_override(self, timeout):
        self.deploy_antenna_with_override(1, timeout)

    # antenna icd section 6.2.10
    @i2cMock.command([0xBB])
    def _deploy_antenna_2_with_override(self, timeout):
        self.deploy_antenna_with_override(2, timeout)

    # antenna icd section 6.2.11
    @i2cMock.command([0xBC])
    def _deploy_antenna_3_with_override(self, timeout):
        self.deploy_antenna_with_override(3, timeout)

    # antenna icd section 6.2.12
    @i2cMock.command([0xBD])
    def _deploy_antenna_4_with_override(self, timeout):
        self.deploy_antenna_with_override(4, timeout)

    # antenna icd section 6.2.8
    @i2cMock.command([0xA5])
    def _deploy_automatically(self, timeout):
        self.log.debug("Beginning automatic antenna deployment on controller")
        self.deployment_in_progress = call(self.on_begin_deployment, True, self, -1)

    def _get_antenna_activation_count(self, antenna_id):
        return [self.antenna_state[antenna_id - 1].activation_count]


    # antenna icd section 6.2.16
    @i2cMock.command([0xB0])
    def _get_antenna_1_activation_count(self):
        return self._get_antenna_activation_count(1)

    # antenna icd section 6.2.17
    @i2cMock.command([0xB1])
    def _get_antenna_2_activation_count(self):
        return self._get_antenna_activation_count(2)

    # antenna icd section 6.2.18
    @i2cMock.command([0xB2])
    def _get_antenna_3_activation_count(self):
        return self._get_antenna_activation_count(3)

    # antenna icd section 6.2.19
    @i2cMock.command([0xB3])
    def _get_antenna_4_activation_count(self):
        return self._get_antenna_activation_count(4)

    def _get_activation_time(self, antennaId):
        time = self.antenna_state[antennaId].activation_time
        return [higher_byte(time), lower_byte(time)]

    # antenna icd section 6.2.20
    @i2cMock.command([0xB4])
    def _get_antenna_1_activation_time(self):
        return self._get_activation_time(0)

    # antenna icd section 6.2.21
    @i2cMock.command([0xB5])
    def _get_antenna_2_activation_time(self):
        return self._get_activation_time(1)

    # antenna icd section 6.2.22
    @i2cMock.command([0xB6])
    def _get_antenna_3_activation_time(self):
        return self._get_activation_time(2)

    # antenna icd section 6.2.23
    @i2cMock.command([0xB7])
    def _get_antenna_4_activation_time(self):
        return self._get_activation_time(3)

    # antenna icd section 6.2.14
    @i2cMock.command([0xC0])
    def _get_temperature(self):
        result = call(self.on_get_temperature, 0)
        return [higher_byte(result), lower_byte(result)]

    @staticmethod
    def update_value(value, condition, flag):
        if condition:
            return value | flag
        else:
            return value;

    # antenna icd section 6.2.15
    @i2cMock.command([0xC3])
    def _get_deployment_status(self):
        result = call(self.on_get_deployment_status, None)
        if not result is None:
            return result

        msb = self.update_value(0, not self.antenna_state[0].is_deployed(), 0x80)
        msb = self.update_value(msb, self.antenna_state[0].deployment_in_progress(), 0x20)
        msb = self.update_value(msb, not self.antenna_state[1].is_deployed(), 0x8)
        msb = self.update_value(msb, self.antenna_state[1].deployment_in_progress(), 0x2)
        msb = self.update_value(msb, self.ignore_deployment_switch, 1)
        lsb = self.update_value(0, not self.antenna_state[1].is_deployed(), 0x80)
        lsb = self.update_value(lsb, self.antenna_state[2].deployment_in_progress(), 0x20)
        lsb = self.update_value(lsb, not self.antenna_state[3].is_deployed(), 0x8)
        lsb = self.update_value(lsb, self.antenna_state[3].deployment_in_progress(), 0x2)
        lsb = self.update_value(lsb, self.deployment_in_progress, 0x10)
        lsb = self.update_value(lsb, self.armed, 0x1)
        return [msb, lsb]

    # antenna icd section 6.2.13
    @i2cMock.command([0xA9])
    def _cancel_deployment(self):
        self.ignore_deployment_switch = False
        self.deployment_in_progress = False
        for antenna in self.antenna_state:
            antenna.cancel_deployment()
        call(self.on_deployment_cancel, None)
        self.log.debug("Antenna deployment canceled on controller")

    def finish_antenna_deployment(self, antenna_id):
        if self.antenna_state[antenna_id].is_deployed():
            self.antenna_state[antenna_id].end_deployment()

    def begin_deployment(self):
        for antenna in self.antenna_state:
            antenna.begin_deployment()

    def finish_deployment(self):
        if self.deployment_in_progress:
            self.deployment_in_progress = False
            for antenna in self.antenna_state:
                antenna.end_deployment()

        for antenna in self.antenna_state:
            if antenna.deployment_in_progress():
                antenna.end_deployment()

        self.ignore_deployment_switch = False
        self.log.debug("Antenna deployment finished on controller")
