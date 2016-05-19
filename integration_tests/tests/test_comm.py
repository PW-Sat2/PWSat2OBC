import devices
from tests.base import BaseTest


class Test_Comm(BaseTest):
    def test_should_initialize_transmitter(self):
        self.assertTrue(self.system.transmitter.wait_for_reset(3))

    def test_should_send_frame(self):
        self.system.obc.send_frame("ABC")
        msg = self.system.transmitter.get_message_from_buffer(3)

        self.assertEqual(msg, (65, 66, 67))

    def test_should_initialize_receiver(self):
        self.assertTrue(self.system.receiver.wait_for_reset(3))

    def test_should_get_number_of_frames(self):
        self.system.receiver.put_frame("ABC")

        count = int(self.system.obc.get_frame_count())

        self.assertEqual(count, 1)

    def test_should_receive_frame(self):
        self.system.receiver.put_frame("ABC")

        frame = self.system.obc.receive_frame()

        self.assertEqual(frame, "ABC")

    def test_should_remove_frame_after_receive(self):
        self.system.receiver.put_frame("ABC")

        self.system.obc.receive_frame()

        self.assertEqual(self.system.receiver.queue_size(), 0)

    def test_should_receive_biggest_possible_frame(self):
        frame = "".join([chr(ord('A') + i % 25) for i in xrange(0, devices.TransmitterDevice.MAX_CONTENT_SIZE)])

        self.system.receiver.put_frame(frame)

        received_frame = self.system.obc.receive_frame()

        self.assertEqual(received_frame, frame)

    def test_build_receive_frame_response(self):
        data = "a" * 300
        doppler = 412
        rssi = 374

        response = devices.ReceiverDevice.build_frame_response(data, doppler, rssi)

        self.assertEqual(response[0:2], [0x2C, 0x01], "Length")
        self.assertEqual(response[2:4], [0x9C, 0x01], "Doppler")
        self.assertEqual(response[4:6], [0x76, 0x01], "RSSI")
        self.assertEqual(response[6:307], [ord('a')] * 300)

        print devices.ReceiverDevice.build_frame_response("ABC", 300, 320)
