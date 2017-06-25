import struct
from base import BaseTest
from system import runlevel
from utils import TestEvent

class PayloadDriverTest(BaseTest):
    def decode_result(self, result_string):

        if "failed" in result_string:
            return None

        lines = result_string.split('\n')
        data = lines[1].split(' ')
        result = []
        for value in data:
            if '0x' not in value:
                result.append(int(value))

        return result

    @runlevel(1)
    def test_payload_whoami(self):

        result = self.system.obc.payload_whoami()
        data = self.decode_result(result)

        self.assertEqual(data, [self.system.payload.whoami])

    @runlevel(1)
    def test_payload_suns(self):

        result = self.system.obc.payload_suns()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.SunS_Ref)

    @runlevel(1)
    def test_payload_temperatures(self):

        result = self.system.obc.payload_temps()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.Temperatures)

    @runlevel(1)
    def test_payload_photodiodes(self):
        result = self.system.obc.payload_photodiodes()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.Photodiodes)

    @runlevel(1)
    def test_payload_housekeeping(self):
        result = self.system.obc.payload_housekeeping()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.Housekeeping)

    @runlevel(1)
    def test_payload_radfet_on(self):
        result = self.system.obc.payload_radfet_on()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.RadFET)

    @runlevel(1)
    def test_payload_radfet_read(self):
        result = self.system.obc.payload_radfet_read()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.RadFET)

    @runlevel(1)
    def test_payload_radfet_off(self):
        result = self.system.obc.payload_radfet_off()
        data = self.decode_result(result)

        self.assertEqual(data, self.system.payload.RadFET)

    @runlevel(1)
    def test_payload_radfet_full_test(self):
        self.system.payload.set_long_timeout(5)

        result = self.system.obc.payload_radfet_on()
        data = self.decode_result(result)
        self.assertEqual(data, self.system.payload.RadFET)

        result = self.system.obc.payload_radfet_read()
        data = self.decode_result(result)
        self.assertEqual(data, self.system.payload.RadFET)

        result = self.system.obc.payload_radfet_read()
        data = self.decode_result(result)
        self.assertEqual(data, self.system.payload.RadFET)

        result = self.system.obc.payload_radfet_off()
        data = self.decode_result(result)
        self.assertEqual(data, self.system.payload.RadFET)
