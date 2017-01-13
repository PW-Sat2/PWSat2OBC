from unittest import TestCase

import devices
from devices import DownlinkFrame, UplinkFrame
from utils import ensure_byte_list


class CommSupportTest(TestCase):
    def test_parse_downlink_frame(self):
        bytes = '\x04\x00\x00PONG'

        frame = DownlinkFrame.parse(bytes)

        self.assertIsNotNone(frame)
        self.assertEqual(frame.apid(), 0x01)
        self.assertEqual(frame.seq(), 0x0)
        self.assertEqual(frame.payload(), ensure_byte_list('PONG'))

    def test_build_uplink_frame(self):
        apid = 12
        content = 'ABC'

        frame = UplinkFrame(apid, content, security_code=0xAABBCCDD)

        b = frame.build()

        self.assertEqual(b[0:4], [0xAA, 0xBB, 0xCC, 0xDD])
        self.assertEqual(b[4], 12)
        self.assertEqual(b[5:8], [65, 66, 67])

    def test_build_receive_frame_response(self):
        data = "a" * 300
        doppler = 412
        rssi = 374

        response = devices.ReceiverDevice.build_frame_response(data, doppler, rssi)

        self.assertEqual(response[0:2], [0x2C, 0x01], "Length")
        self.assertEqual(response[2:4], [0x9C, 0x01], "Doppler")
        self.assertEqual(response[4:6], [0x76, 0x01], "RSSI")
        self.assertEqual(response[6:307], [ord('a')] * 300)
