import devices
from tests.base import BaseTest
from system import auto_comm_handling, auto_power_on
from utils import TestEvent
from obc import *

class Test_Comm(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName = 'runTest'):
        super(Test_Comm, self).__init__(methodName)

    def suspend_comm(self):
        self.system.obc.comm_auto_handling(False)

    def startup(self):
        self.power_on_and_wait()
        self.suspend_comm()

    def module_reset(self, module, event):
        self.startup()
        self.system.obc.comm_reset(module)
        self.assertTrue(event.wait_for_change(1))

    def test_should_initialize_hardware(self):
        event = TestEvent()
        self.system.comm.on_hardware_reset = event.set
        self.system.obc.power_on()
        self.assertTrue(event.wait_for_change(1))

    def test_transmitter_reset(self):
        event = TestEvent()
        self.system.comm.transmitter.on_reset = event.set
        self.module_reset(CommModule.Transmitter, event)

    def test_receiver_reset(self):
        event = TestEvent()
        self.system.comm.receiver.on_reset = event.set
        self.module_reset(CommModule.Receiver, event)

    def test_hardware_reset(self):
        event = TestEvent()
        self.system.comm.on_hardware_reset = event.set
        self.module_reset(CommModule.Both, event)

    def test_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.module_reset(CommModule.Watchdog, event)

    def test_receiver_reset(self):
        event = TestEvent()
        self.system.comm.receiver.on_reset = event.set
        self.startup()
        self.system.obc.comm_reset(CommModule.Receiver)
        self.assertTrue(event.wait_for_change(1))

    def test_should_send_frame(self):
        self.startup()
        self.system.obc.send_frame("ABC")
        msg = self.system.transmitter.get_message_from_buffer(3)
        self.assertEqual(msg, (65, 66, 67))

    def test_should_get_number_of_frames(self):
        self.startup()
        self.system.receiver.put_frame("ABC")

        count = int(self.system.obc.get_frame_count())

        self.assertEqual(count, 1)

    def test_should_receive_frame(self):
        self.startup()
        self.system.receiver.put_frame("ABC")

        frame = self.system.obc.receive_frame()

        self.assertEqual(frame, "ABC")

    def test_should_remove_frame_after_receive(self):
        event = TestEvent()
        self.system.receiver.on_frame_remove = event.set
        self.startup()
        self.system.receiver.put_frame("ABD")

        self.system.obc.receive_frame()
        self.assertTrue(event.wait_for_change(1))

        self.assertEqual(self.system.receiver.queue_size(), 0)

    def test_should_receive_biggest_possible_frame(self):
        self.startup()
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

    def test_auto_pingpong(self):
        self.power_on_and_wait()
        self.system.receiver.put_frame("PING")
        msg = self.system.transmitter.get_message_from_buffer(25)

        msg = ''.join([chr(c) for c in msg])

        self.assertEqual(msg, "PONG")

    def test_auto_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.power_on_and_wait()
        self.assertTrue(event.wait_for_change(5))
