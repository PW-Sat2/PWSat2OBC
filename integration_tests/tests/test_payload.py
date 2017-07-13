from system import runlevel
from tests.base import RestartPerSuite


@runlevel(1)
class PayloadDriverTest(RestartPerSuite):
    def test_payload_whoami(self):
        result = self.system.obc.payload_whoami()
        self.assertEqual(result['Who Am I'], self.system.payload.Status.WhoAmI)

    def test_payload_suns(self):
        result = self.system.obc.payload_suns()
        self.assertEqual(result['V1'], self.system.payload.SunS_Ref.V1)
        self.assertEqual(result['V2'], self.system.payload.SunS_Ref.V2)
        self.assertEqual(result['V3'], self.system.payload.SunS_Ref.V3)
        self.assertEqual(result['V4'], self.system.payload.SunS_Ref.V4)
        self.assertEqual(result['V5'], self.system.payload.SunS_Ref.V5)

    def test_payload_temperatures(self):
        result = self.system.obc.payload_temps()
        self.assertEqual(result['Supply'], self.system.payload.Temperatures.Supply)
        self.assertEqual(result['Xp'], self.system.payload.Temperatures.Xp)
        self.assertEqual(result['Xn'], self.system.payload.Temperatures.Xn)
        self.assertEqual(result['Yp'], self.system.payload.Temperatures.Yp)
        self.assertEqual(result['Yn'], self.system.payload.Temperatures.Yn)
        self.assertEqual(result['SADS'], self.system.payload.Temperatures.SADS)
        self.assertEqual(result['Sail'], self.system.payload.Temperatures.Sail)
        self.assertEqual(result['CAM Nadir'], self.system.payload.Temperatures.CAMNadir)
        self.assertEqual(result['CAM Wing'], self.system.payload.Temperatures.CAMWing)

    def test_payload_photodiodes(self):
        result = self.system.obc.payload_photodiodes()
        self.assertEqual(result['Xp'], self.system.payload.Photodiodes.Xp)
        self.assertEqual(result['Xn'], self.system.payload.Photodiodes.Xn)
        self.assertEqual(result['Yp'], self.system.payload.Photodiodes.Yp)
        self.assertEqual(result['Yn'], self.system.payload.Photodiodes.Yn)

    def test_payload_housekeeping(self):
        result = self.system.obc.payload_housekeeping()
        self.assertEqual(result['INT 3V3D'], self.system.payload.Housekeeping.INT_3V3D)
        self.assertEqual(result['OBC 3V3D'], self.system.payload.Housekeeping.OBC_3V3D)

    def check_radfet_result(self, result):
        self.assertEqual(result['Status'], self.system.payload.RadFET.Status)
        self.assertEqual(result['Temperature'], self.system.payload.RadFET.Temperature)
        self.assertEqual(result['Vth0'], self.system.payload.RadFET.Vth0)
        self.assertEqual(result['Vth1'], self.system.payload.RadFET.Vth1)
        self.assertEqual(result['Vth2'], self.system.payload.RadFET.Vth2)

    def test_payload_radfet_on(self):
        result = self.system.obc.payload_radfet_on()
        self.check_radfet_result(result)

    def test_payload_radfet_read(self):
        result = self.system.obc.payload_radfet_read()
        self.check_radfet_result(result)

    def test_payload_radfet_off(self):
        result = self.system.obc.payload_radfet_off()
        self.check_radfet_result(result)

    def test_payload_radfet_full_test(self):
        result = self.system.obc.payload_radfet_on()
        self.check_radfet_result(result)

        self.system.payload.set_timeout_callback(lambda: 5)
        result = self.system.obc.payload_radfet_read()
        self.check_radfet_result(result)

        result = self.system.obc.payload_radfet_read()
        self.check_radfet_result(result)

        self.system.payload.set_timeout_callback()
        result = self.system.obc.payload_radfet_off()
        self.check_radfet_result(result)
