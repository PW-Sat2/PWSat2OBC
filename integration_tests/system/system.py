import logging

from devices import *
from i2cMock import I2CMock, MockPin
from obc import OBC, SerialPortTerminal
import response_frames
from obc.boot import BootHandler, BootToUpper, BootToIndex
from pins import Pins


class System:
    def __init__(self, obc_com, mock_com, gpio, final_boot_handler):
        self.log = logging.getLogger("system")

        self.obc_com = obc_com
        self.mock_com = mock_com

        self.i2c = I2CMock(mock_com)

        self._setup_devices()

        self.obc = OBC(SerialPortTerminal(obc_com, gpio))

        self._final_boot_handler = final_boot_handler

    def start(self):
        self.i2c.start()

    def _setup_devices(self):
        self.frame_decoder = response_frames.FrameDecoder(response_frames.frame_factories)

        self.eps = EPS()
        self.comm = Comm(self.frame_decoder)
        self.transmitter = self.comm.transmitter
        self.receiver = self.comm.receiver
        self.primary_antenna = AntennaController(PRIMARY_ANTENNA_CONTROLLER_ADDRESS, "Primary Antenna")
        self.backup_antenna = AntennaController(BACKUP_ANTENNA_CONTROLLER_ADDRESS, "Backup Antenna")
        self.imtq = Imtq()
        self.suns = SunS(GPIODriver(self.i2c), MockPin.PA9)
        self.gyro = Gyro()
        self.rtc = RTCDevice()
        self.payload = Payload(GPIODriver(self.i2c), MockPin.PA8)
        self.camera = CameraDriver(self.i2c, MockPin.PC10, MockPin.PC12)

        self.i2c.add_bus_device(self.eps.controller_a)
        self.i2c.add_pld_device(self.eps.controller_b)
        self.i2c.add_bus_device(self.transmitter)
        self.i2c.add_bus_device(self.receiver)
        self.i2c.add_bus_device(self.primary_antenna)
        self.i2c.add_pld_device(self.backup_antenna)
        self.i2c.add_bus_device(self.imtq)
        self.i2c.add_pld_device(self.suns)
        self.i2c.add_pld_device(self.rtc)
        self.i2c.add_pld_device(self.gyro)
        self.i2c.add_pld_device(self.payload)

    def close(self):
        self.i2c.stop()
        self.obc.close()

    def restart(self, boot_chain=None):
        if boot_chain is None:
            boot_chain = []

        self.i2c.unlatch()
        self.obc.reset(boot_handler=BootHandler(boot_chain + [self._final_boot_handler]))
        self.obc.wait_to_start()

    def power_off_obc(self):
        self.obc.power_off()

    @classmethod
    def build_from_config(cls, config):
        obc_com = config['OBC_COM']
        mock_com = config['MOCK_COM']
        gpio_com = config['GPIO_COM']
        boot_handler = BootToUpper() if config['BOOT_UPPER'] else BootToIndex(config['BOOT_INDEX'])

        gpio = Pins(gpio_com)

        system = System(obc_com, mock_com, gpio, boot_handler)

        return gpio, system
