import i2cMock
from i2cMock import I2CDevice

EPS_DEVICE_ADDRESS = 12

class EPSDevice(I2CDevice):
    def __init__(self):
        self.opened = False
        return super(EPSDevice, self).__init__(EPS_DEVICE_ADDRESS)

    @i2cMock.command([0x01])
    def lcl_sail_0(self, onoff):
        self.opened = True
        print "LCL_SAIL_0: %s" % str(onoff)

    @i2cMock.command([0x02])
    def lcl_sail_1(self, onoff):
        print "LCL_SAIL_1: %s" % str(onoff)