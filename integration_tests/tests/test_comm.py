import devices
from obc import *
from system import auto_power_on
from tests.base import BaseTest
from utils import TestEvent, ensure_byte_list


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

        self.assertEqual(frame[0:], "ABC")

    def test_should_remove_frame_after_receive(self):
        event = TestEvent()
        self.system.receiver.on_frame_remove = event.set
        self.startup()
        self.system.receiver.put_frame("ABD")

        self.system.obc.receive_frame()
        self.assertTrue(event.wait_for_change(1))

        self.assertEqual(self.system.receiver.queue_size(), 0)

    def test_auto_pingpong(self):
        def reset_handler(*args):
            return False
        
        self.system.comm.on_hardware_reset = reset_handler
        self.system.comm.receiver.on_reset = reset_handler
        self.power_on_and_wait()

        self.system.comm.put_frame(devices.UplinkFrame(ord('P'), 'ABC'))

        msg = self.system.comm.get_frame(20)

        self.assertEqual(msg.payload(), ensure_byte_list("PONG"))

    def test_auto_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.power_on_and_wait()
        self.assertTrue(event.wait_for_change(5))
