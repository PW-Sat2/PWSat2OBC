import logging
import threading
import i2cMock
from i2cMock import I2CDevice
import inspect

class Antenna():
    deployed = False
    activation_count = 0
    is_being_deployed = False

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

PRIMARY_ANTENNA_CONTROLLER_ADDRESS = 0x32
BACKUP_ANTENNA_CONTROLLER_ADDRESS = 0x34

class AntennaController(i2cMock.I2CDevice):
    armed = False
    ignore_deployment_switch = False
    antenna_state = [Antenna(), Antenna(), Antenna(), Antenna()]
    deployment_in_progress = False

    on_reset = None
    on_arm_state_change = None
    on_begin_deployment = None
    on_get_temperature = None
    on_get_deployment_status = None
    on_deployment_cancel = None

    def __init__(self, address):
        super(AntennaController, self).__init__(address)
        self.log = logging.getLogger("Antenna")

    def reset_state(self):
        self.armed = False
        self.ignore_deployment_switch = False
        self.antenna_state = [Antenna(), Antenna(), Antenna(), Antenna()]
        self.deployment_in_progress = False

    @staticmethod
    def call(method, default, *args):
        if not method is None:
            if inspect.ismethod(method):
                if len(args) != (method.__code__.co_argcount - 1):
                    raise Exception("Invalid argument count")
            else:
                if len(args) != method.__code__.co_argcount:
                    raise Exception("Invalid argument count")

            result = method(*args)
            if not result is None:
                return result

        return default

    @i2cMock.command([0xAA])
    def reset(self):
        self.log.debug("Resetting antenna controller: %d", self.address)
        self.reset_state()
        self.call(self.on_reset, None)

    @i2cMock.command([0xAD])
    def arm_deployment_system(self):
        self.log.debug("Arming deployment system controller: %d", self.address)
        if not self.armed:
            self.armed = self.call(self.on_arm_state_change, True, True)

    @i2cMock.command([0xAC])
    def disarm_deployment_system(self):
        self.log.debug("Disarming deployment system controller: %d", self.address)
        if self.armed:
            self.armed = self.call(self.on_arm_state_change, False, False)

    def deploy_antenna(self, antanna_id, timeout):
        self.log.debug("Beginning of deployment antenna %d on controller: %d", antanna_id + 1, self.address)
        self.antenna_state[antanna_id].begin_deployment()
        self.call(self.on_begin_deployment, None, self, antanna_id)

    @i2cMock.command([0xA1])
    def deploy_antanna_1(self, timeout):
        self.deploy_antenna(1, timeout)

    @i2cMock.command([0xA2])
    def deploy_antanna_2(self, timeout):
        self.deploy_antenna(2, timeout)

    @i2cMock.command([0xA3])
    def deploy_antanna_3(self, timeout):
        self.deploy_antenna(3, timeout)

    @i2cMock.command([0xA4])
    def deploy_antanna_4(self, timeout):
        self.deploy_antenna(4, timeout)

    def deploy_antenna_with_override(self, antanna_id, timeout):
        self.ignore_deployment_switch = True
        self.deploy_antenna(antanna_id, timeout)

    @i2cMock.command([0xBA])
    def deploy_antanna_1_with_override(self, timeout):
        self.deploy_antenna_with_override(1, timeout)

    @i2cMock.command([0xBB])
    def deploy_antanna_2_with_override(self, timeout):
        self.deploy_antenna_with_override(2, timeout)

    @i2cMock.command([0xBC])
    def deploy_antanna_3_with_override(self, timeout):
        self.deploy_antenna_with_override(3, timeout)

    @i2cMock.command([0xBD])
    def deploy_antanna_4_with_override(self, timeout):
        self.deploy_antenna_with_override(4, timeout)

    @i2cMock.command([0xA5])
    def deploy_automatically(self, timeout):
        self.log.debug("Beginning automatic antenna deployment on controller: %d", self.address)
        self.deployment_in_progress = True
        self.call(self.on_begin_deployment, None, self, -1)

    def get_antenna_activation_count(self, antenna_id):
        return [self.antenna_state[antenna_id].activation_count]

    @i2cMock.command([0xB0])
    def get_antenna_1_activation_count(self):
        return self.get_antenna_activation_count(0)

    @i2cMock.command([0xB1])
    def get_antenna_1_activation_count(self):
        return self.get_antenna_activation_count(1)

    @i2cMock.command([0xB2])
    def get_antenna_1_activation_count(self):
        return self.get_antenna_activation_count(2)

    @i2cMock.command([0xB3])
    def get_antenna_1_activation_count(self):
        return self.get_antenna_activation_count(3)

    @i2cMock.command([0xB4])
    def get_antenna_1_activation_time(self):
        return [0, 0]

    @i2cMock.command([0xB5])
    def get_antenna_1_activation_time(self):
        return [0, 0]

    @i2cMock.command([0xB6])
    def get_antenna_1_activation_time(self):
        return [0, 0]

    @i2cMock.command([0xB7])
    def get_antenna_1_activation_time(self):
        return [0, 0]

    @i2cMock.command([0xC0])
    def get_temperature(self):
        result = self.call(self.on_get_temperature, None)
        if result is None:
            result = 0

        return [(result >> 8) & 0xff, result & 0xff]

    @staticmethod
    def update_value(value, condition, flag):
        if condition:
            return value | flag
        else:
            return value;

    @i2cMock.command([0xC3])
    def get_deployment_status(self):
        result = self.call(self.on_get_deployment_status, None)
        if not result is None:
            return result

        msb = update_value(0, not self.antenna_state[0].is_deployed(), 0x80)
        msb = update_value(msb, self.antenna_state[0].is_being_deployed(), 0x20)
        msb = update_value(msb, not self.antenna_state[1].is_deployed(), 0x8)
        msb = update_value(msb, self.antenna_state[1].is_being_deployed(), 0x2)
        msb = update_value(msb, self.ignore_deployment_switch, 1)
        lsb = update_value(0, not self.antenna_state[1].is_deployed(), 0x80)
        lsb = update_value(lsb, self.antenna_state[2].is_being_deployed(), 0x20)
        lsb = update_value(lsb, not self.antenna_state[3].is_deployed(), 0x8)
        lsb = update_value(lsb, self.antenna_state[3].is_being_deployed(), 0x2)
        lsb = update_value(lsb, self.deployment_in_progress, 0x10)
        lsb = update_value(lsb, self.armed, 0x1)
        return [msb, lsb]

    @i2cMock.command([0xA9])
    def cancel_deployment(self):
        self.ignore_deployment_switch = False
        self.deployment_in_progress = False
        for antenna in self.antenna_state:
            antenna.cancel_deployment()
        self.call(self.on_deployment_cancel, None)
        self.log.debug("Antenna deployment canceled on controller: %d", self.address)

    def finish_antenna_deployment(self, antenna_id):
        if self.antenna_state[antenna_id].is_deployed():
            self.antenna_state[antenna_id].end_deployment()

    def finish_deployment(self):
        if self.deployment_in_progress:
            self.deployment_in_progress = False
            for antenna in self.antenna_state:
                antenna.end_deployment()

        for antenna in self.antenna_state:
            if antenna.is_deployed():
                antenna.end_deployment()

        self.ignore_deployment_switch = False
        self.log.debug("Antenna deployment finished on controller: %d", self.address)
