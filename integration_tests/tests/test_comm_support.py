from unittest import TestCase

import devices
from devices import DownlinkFrame, UplinkFrame
from response_frames.beacon_factory import BeaconFrameFactory
from response_frames.downlink_frame_factory import ResponseFrame, response_frame, DownlinkFrameFactory
from response_frames.marker import BeaconMarker
from utils import ensure_byte_list

@response_frame(0x01)
class Frame1(ResponseFrame):
    def __init__(self, apid, seq, payload):
        super(Frame1, self).__init__(apid, seq, payload)

    @classmethod
    def matches(cls, payload):
        if len(payload) != 4:
            return False

        return payload[0] == 1

@response_frame(0x02)
class Frame2(ResponseFrame):
    def __init__(self, apid, seq, payload):
        super(Frame2, self).__init__(apid, seq, payload)

    @classmethod
    def matches(cls, payload):
        if len(payload) != 4:
            return False

        return payload[0] == 1

@response_frame(0x02)
class Frame3(ResponseFrame):
    def __init__(self, apid, seq, payload):
        super(Frame3, self).__init__(apid, seq, payload)

    @classmethod
    def matches(cls, payload):
        if len(payload) != 4:
            return False

        return payload[0] == 4

    def decode(self):
        self.x = self.payload()[1]

class CommSupportTest(TestCase):
    def test_parse_downlink_frame(self):
        bytes = '\x42\xD5\x76PONG'

        frame = DownlinkFrame.parse(bytes)

        self.assertIsNotNone(frame)
        self.assertEqual(frame.apid(), 0x02)
        self.assertEqual(frame.seq(), 0x1DB55)
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

    def test_beacon_factory_recognizes_beacon_frames(self):
        decoder = BeaconFrameFactory()
        self.assertTrue(decoder.matches([BeaconMarker(), 0]))
    
    def test_beacon_factory_ignores_non_beacon_frames(self):
        decoder = BeaconFrameFactory()
        self.assertFalse(decoder.matches([BeaconMarker() - 1, 0]))
        self.assertFalse(decoder.matches([BeaconMarker() + 1, 0]))

    def test_downlink_frame_factory_ignores_beacon_frames(self):
        decoder = DownlinkFrameFactory([Frame2, Frame1, Frame3])
        self.assertFalse(decoder.matches([BeaconMarker(), 0]))

    def test_downlink_frame_factory_captures_non_beacon_frames(self):
        decoder = DownlinkFrameFactory([Frame2, Frame1, Frame3])
        self.assertTrue(decoder.matches([BeaconMarker() - 1, 0]))
        self.assertTrue(decoder.matches([BeaconMarker() + 1, 0]))

    def test_decode_downlink_frame(self):
        decoder = DownlinkFrameFactory([Frame2, Frame1, Frame3])

        frame = DownlinkFrame(0x01, 0x02, [1, 2, 3, 4])
        decoded = decoder.decode_frame(frame)

        self.assertIsInstance(decoded, Frame1)

        frame = DownlinkFrame(0x02, 0x02, [4, 3, 2, 1])
        decoded = decoder.decode_frame(frame)

        self.assertIsInstance(decoded, Frame3)
        self.assertEqual(decoded.x, 3)
