import struct
import i2cMock
from i2cMock import I2CDevice
from threading import Timer


class Payload(I2CDevice):
    def __init__(self, gpio_driver, pin):
        super(Payload, self).__init__(0b0110000, "Payload")

        self.interrupt_pin = pin
        self.gpioDriver = gpio_driver
        self.Status = self.StatusTelemetry()
        self.SunS_Ref = self.SunSTelemetry()
        self.Temperatures = self.TemperatureTelemetry()
        self.Photodiodes = self.PhotodiodeTelemetry()
        self.Housekeeping = self.HousekeepingTelemetry()
        self.RadFET = self.RadFETTelemetry()

        # processing hooks
        self.timeout_callback = None
        self.external_measure_suns_ref_callback = None
        self.external_measure_temperatures_callback = None
        self.external_measure_photodiodes_callback = None
        self.external_measure_housekeeping_callback = None
        self.external_initialize_radfet_callback = None
        self.external_measure_radfet_callback = None
        self.external_finish_radfet_callback = None

    # Interrupt pin handling

    def gpio_interrupt_low(self):
        self.gpioDriver.gpio_low(self.interrupt_pin)

    def gpio_interrupt_high(self):
        self.gpioDriver.gpio_high(self.interrupt_pin)

    # Commands

    @i2cMock.command([0x80])
    def measure_suns(self):
        if self.external_measure_suns_ref_callback:
            self.external_measure_suns_ref_callback()
        else:
            self.mock_measure_suns_ref()
            self.mock_processing_start()

    @i2cMock.command([0x81])
    def measure_temperatures(self):
        if self.external_measure_temperatures_callback:
            self.external_measure_temperatures_callback()
        else:
            self.mock_measure_temperatures()
            self.mock_processing_start()

    @i2cMock.command([0x82])
    def measure_photodiodes(self):
        if self.external_measure_photodiodes_callback:
            self.external_measure_photodiodes_callback()
        else:
            self.mock_measure_photodiodes()
            self.mock_processing_start()

    @i2cMock.command([0x83])
    def measure_housekeeping(self):
        if self.external_measure_housekeeping_callback:
            self.external_measure_housekeeping_callback()
        else:
            self.mock_measure_housekeeping()
            self.mock_processing_start()

    @i2cMock.command([0x84])
    def radfet_on(self):
        if self.external_initialize_radfet_callback:
            self.external_initialize_radfet_callback()
        else:
            self.mock_initialize_radfet()
            self.mock_processing_start()

    @i2cMock.command([0x85])
    def measure_radfet(self):
        if self.external_measure_radfet_callback:
            self.external_measure_radfet_callback()
        else:
            self.mock_measure_radfet()
            self.mock_processing_start()

    @i2cMock.command([0x86])
    def radfet_off(self):
        if self.external_finish_radfet_callback:
            self.external_finish_radfet_callback()
        else:
            self.mock_finish_radfet()
            self.mock_processing_start()

    # Data Reads

    @i2cMock.command([0x00])
    def read_whoami(self):
        return self.Status.get()

    @i2cMock.command([0x01])
    def read_suns_ref(self):
        return self.SunS_Ref.get()

    @i2cMock.command([11])
    def read_temperatures(self):
        return self.Temperatures.get()

    @i2cMock.command([29])
    def read_photodiodes(self):
        return self.Photodiodes.get()

    @i2cMock.command([37])
    def read_housekeeping(self):
        return self.Housekeeping.get()

    @i2cMock.command([41])
    def read_radfet(self):
        return self.RadFET.get()

    # Value setters

    def set_suns_ref(self, new_value):
        self.SunS_Ref.set(new_value)

    def set_temperatures(self, new_value):
        self.Temperatures.set(new_value)

    def set_photodiodes(self, new_value):
        self.Photodiodes.set(new_value)

    def set_housekeeping(self, new_value):
        self.Housekeeping.set(new_value)

    def set_radfet(self, new_value):
        self.RadFET.set(new_value)

    def set_whoami(self, new_value):
        self.Status.set(new_value)

    def set_timeout_callback(self, new_timeout_callback=None):
        self.timeout_callback = new_timeout_callback

    def set_external_measure_suns_ref_callback(self, new_value=None):
        self.external_measure_suns_ref_callback = new_value

    def set_external_measure_temperatures_callback(self, new_value=None):
        self.external_measure_temperatures_callback = new_value

    def set_external_measure_photodiodes_callback(self, new_value=None):
        self.external_measure_photodiodes_callback = new_value

    def set_external_measure_housekeeping_callback(self, new_value=None):
        self.external_measure_housekeeping_callback = new_value

    def set_external_initialize_radfet_callback(self, new_value=None):
        self.external_initialize_radfet_callback = new_value

    def set_external_measure_radfet_callback(self, new_value=None):
        self.external_measure_radfet_callback = new_value

    def set_external_finish_radfet_callback(self, new_value=None):
        self.external_finish_radfet_callback = new_value

    # Measurements mock

    def mock_processing_start(self):
        self.gpio_interrupt_high()
        self.log.info("Start PLD measurement")

        timeout = 0
        if self.timeout_callback is not None:
            timeout = self.timeout_callback()
        t = Timer(timeout, self.mock_processing_stop)
        t.start()

    def mock_processing_stop(self):
        self.log.info("Finished PLD measurement")
        self.gpio_interrupt_low()

    def mock_measure_suns_ref(self):
        if self.SunS_Ref.is_empty():
            self.set_suns_ref({'V1': 101, 'V2': 102, 'V3': 103, 'V4': 104, 'V5': 105})

    def mock_measure_temperatures(self):
        if self.Temperatures.is_empty():
            self.set_temperatures({'Supply': 201, 'Xp': 202, 'Xn': 203, 'Yp': 204, 'Yn': 205, 'SADS': 206,
                                   'Sail': 207, 'CAM Nadir': 208, 'CAM Wing': 209})

    def mock_measure_photodiodes(self):
        if self.Photodiodes.is_empty():
            self.set_photodiodes({'Xp': 301, 'Xn': 302, 'Yp': 303, 'Yn': 304})

    def mock_measure_housekeeping(self):
        if self.Housekeeping.is_empty():
            self.set_housekeeping({'INT 3V3D': 401, 'OBC 3V3D': 402})

    def mock_initialize_radfet(self):
        self.set_radfet({'Status': 50, 'Temperature': 500001, 'Vth0': 500002,
                         'Vth1': 500003, 'Vth2': 500004})

    def mock_measure_radfet(self):
        if not self.RadFET.is_empty():
            self.RadFET.Status = self.RadFET.Status + 1
            self.RadFET.Temperature = self.RadFET.Temperature + 100
            self.RadFET.Vth0 = self.RadFET.Vth0 + 100
            self.RadFET.Vth1 = self.RadFET.Vth1 + 100
            self.RadFET.Vth2 = self.RadFET.Vth2 + 100
        else:
            # mock read of unintialized radfet
            self.set_radfet({'Status': 99, 'Temperature': 0x00FFFFFF, 'Vth0': 0x00FFFFFF,
                             'Vth1': 0x00FFFFFF, 'Vth2': 0x00FFFFFF})

    def mock_finish_radfet(self):
        if not self.RadFET.is_empty():
            self.set_radfet({'Status': 59, 'Temperature': 590001, 'Vth0': 590002,
                             'Vth1': 590003, 'Vth2': 590004})
        else:
            # mock read of unintialized radfet
            self.set_radfet({'Status': 111, 'Temperature': 0x00FFFFFF, 'Vth0': 0x00FFFFFF,
                             'Vth1': 0x00FFFFFF, 'Vth2': 0x00FFFFFF})

    # Telemetry classes

    class StatusTelemetry:
        def __init__(self):
            self.WhoAmI = 0x53

        def set(self, telemetry_dict):
            self.WhoAmI = telemetry_dict.get('Who Am I')

        def get(self):
            return list(struct.pack('<B', self.WhoAmI))

    class SunSTelemetry:
        def __init__(self):
            self.V1 = 0xFFFF
            self.V2 = 0xFFFF
            self.V3 = 0xFFFF
            self.V4 = 0xFFFF
            self.V5 = 0xFFFF

        def set(self, telemetry_dict):
            self.V1 = telemetry_dict.get('V1')
            self.V2 = telemetry_dict.get('V2')
            self.V3 = telemetry_dict.get('V3')
            self.V4 = telemetry_dict.get('V4')
            self.V5 = telemetry_dict.get('V5')

        def get(self):
            return list(struct.pack('<HHHHH',
                                    self.V1,
                                    self.V2,
                                    self.V3,
                                    self.V4,
                                    self.V5))

        def is_empty(self):
            return self.V1 == 0xFFFF and self.V2 == 0xFFFF and self.V3 == 0xFFFF \
                   and self.V4 == 0xFFFF and self.V5 == 0xFFFF

    class TemperatureTelemetry:
        def __init__(self):
            self.Supply = 0xFFFF
            self.Xp = 0xFFFF
            self.Xn = 0xFFFF
            self.Yp = 0xFFFF
            self.Yn = 0xFFFF
            self.SADS = 0xFFFF
            self.Sail = 0xFFFF
            self.CAMNadir = 0xFFFF
            self.CAMWing = 0xFFFF

        def set(self, telemetry_dict):
            self.Supply = telemetry_dict.get('Supply')
            self.Xp = telemetry_dict.get('Xp')
            self.Xn = telemetry_dict.get('Xn')
            self.Yp = telemetry_dict.get('Yp')
            self.Yn = telemetry_dict.get('Yn')
            self.SADS = telemetry_dict.get('SADS')
            self.Sail = telemetry_dict.get('Sail')
            self.CAMNadir = telemetry_dict.get('CAM Nadir')
            self.CAMWing = telemetry_dict.get('CAM Wing')

        def get(self):
            return list(struct.pack('<HHHHHHHHH',
                                    self.Supply,
                                    self.Xp,
                                    self.Xn,
                                    self.Yp,
                                    self.Yn,
                                    self.SADS,
                                    self.Sail,
                                    self.CAMNadir,
                                    self.CAMWing))

        def is_empty(self):
            return self.Supply == 0xFFFF and self.Xp == 0xFFFF and self.Xn == 0xFFFF and self.Yp == 0xFFFF \
                   and self.Yn == 0xFFFF and self.SADS == 0xFFFF and self.Sail == 0xFFFF and self.CAMNadir == 0xFFFF \
                   and self.CAMWing == 0xFFFF

    class PhotodiodeTelemetry:
        def __init__(self):
            self.Xp = 0xFFFF
            self.Xn = 0xFFFF
            self.Yp = 0xFFFF
            self.Yn = 0xFFFF

        def set(self, telemetry_dict):
            self.Xp = telemetry_dict.get('Xp')
            self.Xn = telemetry_dict.get('Xn')
            self.Yp = telemetry_dict.get('Yp')
            self.Yn = telemetry_dict.get('Yn')

        def get(self):
            return list(struct.pack('<HHHH',
                                    self.Xp,
                                    self.Xn,
                                    self.Yp,
                                    self.Yn))

        def is_empty(self):
            return self.Xp == 0xFFFF and self.Xn == 0xFFFF and self.Yp == 0xFFFF and self.Yn == 0xFFFF

    class HousekeepingTelemetry:
        def __init__(self):
            self.INT_3V3D = 0xFFFF
            self.OBC_3V3D = 0xFFFF

        def set(self, telemetry_dict):
            self.INT_3V3D = telemetry_dict.get('INT 3V3D')
            self.OBC_3V3D = telemetry_dict.get('OBC 3V3D')

        def get(self):
            return list(struct.pack('<HH',
                        self.INT_3V3D,
                        self.OBC_3V3D))

        def is_empty(self):
            return self.INT_3V3D == 0xFFFF and self.OBC_3V3D == 0xFFFF

    class RadFETTelemetry:
        def __init__(self):
            self.Status = 0xFF
            self.Temperature = 0xFFFFFFFF
            self.Vth0 = 0xFFFFFFFF
            self.Vth1 = 0xFFFFFFFF
            self.Vth2 = 0xFFFFFFFF

        def set(self, telemetry_dict):
            self.Status = telemetry_dict.get('Status')
            self.Temperature = telemetry_dict.get('Temperature')
            self.Vth0 = telemetry_dict.get('Vth0')
            self.Vth1 = telemetry_dict.get('Vth1')
            self.Vth2 = telemetry_dict.get('Vth2')

        def get(self):
            return list(struct.pack('<BLLLL',
                                    self.Status,
                                    self.Temperature,
                                    self.Vth0,
                                    self.Vth1,
                                    self.Vth2))

        def is_empty(self):
            return self.Status == 0xFF and self.Temperature == 0xFFFFFFFF and self.Vth0 == 0xFFFFFFFF \
                   and self.Vth1 == 0xFFFFFFFF and self.Vth2 == 0xFFFFFFFF
