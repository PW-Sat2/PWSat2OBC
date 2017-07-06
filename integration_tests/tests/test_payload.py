from base import BaseTest
from system import runlevel


class PayloadDriverTest(BaseTest):
    @runlevel(1)
    def test_payload_whoami(self):
        result = self.system.obc.payload_whoami()
        self.assertEqual(result['Who Am I'], self.system.payload.whoami)

    @runlevel(1)
    def test_payload_suns(self):
        result = self.system.obc.payload_suns()
        self.assertEqual(result['V1'], self.system.payload.SunS_Ref[0])
        self.assertEqual(result['V2'], self.system.payload.SunS_Ref[1])
        self.assertEqual(result['V3'], self.system.payload.SunS_Ref[2])
        self.assertEqual(result['V4'], self.system.payload.SunS_Ref[3])
        self.assertEqual(result['V5'], self.system.payload.SunS_Ref[4])

    @runlevel(1)
    def test_payload_temperatures(self):
        result = self.system.obc.payload_temps()
        self.assertEqual(result['Supply'], self.system.payload.Temperatures[0])
        self.assertEqual(result['Xp'], self.system.payload.Temperatures[1])
        self.assertEqual(result['Xn'], self.system.payload.Temperatures[2])
        self.assertEqual(result['Yp'], self.system.payload.Temperatures[3])
        self.assertEqual(result['Yn'], self.system.payload.Temperatures[4])
        self.assertEqual(result['SADS'], self.system.payload.Temperatures[5])
        self.assertEqual(result['Sail'], self.system.payload.Temperatures[6])
        self.assertEqual(result['CAM Nadir'], self.system.payload.Temperatures[7])
        self.assertEqual(result['CAM Wing'], self.system.payload.Temperatures[8])

    @runlevel(1)
    def test_payload_photodiodes(self):
        result = self.system.obc.payload_photodiodes()
        self.assertEqual(result['Xp'], self.system.payload.Photodiodes[0])
        self.assertEqual(result['Xn'], self.system.payload.Photodiodes[1])
        self.assertEqual(result['Yp'], self.system.payload.Photodiodes[2])
        self.assertEqual(result['Yn'], self.system.payload.Photodiodes[3])

    @runlevel(1)
    def test_payload_housekeeping(self):
        result = self.system.obc.payload_housekeeping()
        self.assertEqual(result['INT 3V3D'], self.system.payload.Housekeeping[0])
        self.assertEqual(result['OBC 3V3D'], self.system.payload.Housekeeping[1])

    def check_radfet_result(self, result):
        self.assertEqual(result['Status'], self.system.payload.RadFET[0])
        self.assertEqual(result['Temperature'], self.system.payload.RadFET[1])
        self.assertEqual(result['Vth0'], self.system.payload.RadFET[2])
        self.assertEqual(result['Vth1'], self.system.payload.RadFET[3])
        self.assertEqual(result['Vth2'], self.system.payload.RadFET[4])

    @runlevel(1)
    def test_payload_radfet_on(self):
        result = self.system.obc.payload_radfet_on()
        self.check_radfet_result(result)

    @runlevel(1)
    def test_payload_radfet_read(self):
        result = self.system.obc.payload_radfet_read()
        self.check_radfet_result(result)

    @runlevel(1)
    def test_payload_radfet_off(self):
        result = self.system.obc.payload_radfet_off()
        self.check_radfet_result(result)

    @runlevel(1)
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
