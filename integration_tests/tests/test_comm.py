from nose.tools import nottest

from devices import *
from obc import *
from tests.base import BaseTest
from system import auto_power_on
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
        self.startup()
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

        self.system.comm.put_frame(UplinkFrame(ord('P'), 'ABC'))

        msg = self.system.comm.get_frame(20)

        self.assertEqual(msg.payload(), ensure_byte_list("PONG"))

    def test_auto_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.power_on_and_wait()
        self.assertTrue(event.wait_for_change(5))

    def test_set_baud_rate(self):
        event = TestEvent()
        self.system.transmitter.on_set_baudrate = event.set
        self.startup()
        self.system.obc.comm_set_bitrate(BaudRate.BaudRate9600)
        event.wait_for_change(2)
        self.assertEqual(self.system.transmitter.baud_rate, BaudRate.BaudRate9600)

    def test_get_transmitter_telemetry(self):
        def get_telemetry_handler(*args):
            return TransmitterTelemetry.build(0x123, 0x456, 0x789, 0xabc)

        self.system.transmitter.on_get_telemetry = get_telemetry_handler
        self.startup()
        telemetry = self.system.obc.comm_get_transmitter_telemetry()
        self.assertEqual(telemetry.RFReflectedPower, 0x123)
        self.assertEqual(telemetry.AmplifierTemperature, 0x456)
        self.assertEqual(telemetry.RFForwardPower, 0x789)
        self.assertEqual(telemetry.TransmitterCurrentConsumption, 0xabc)

    def test_get_receiver_telemetry(self):
        def get_telemetry_handler(*args):
            return ReceiverTelemetry.build(0x123, 0x456, 0x789, 0xabc, 0xdef, 0x234, 0x567)

        self.system.receiver.on_get_telemetry = get_telemetry_handler
        self.startup()
        telemetry = self.system.obc.comm_get_receiver_telemetry()
        self.assertEqual(telemetry.TransmitterCurrentConsumption, 0x123)
        self.assertEqual(telemetry.ReceiverCurrentConsumption, 0x456)
        self.assertEqual(telemetry.DopplerOffset, 0x789)
        self.assertEqual(telemetry.Vcc, 0xabc)
        self.assertEqual(telemetry.OscilatorTemperature, 0xdef)
        self.assertEqual(telemetry.AmplifierTemperature, 0x234)
        self.assertEqual(telemetry.SignalStrength, 0x567)

    def test_set_idle_state(self):
        enabled = TestEvent()
        disabled = TestEvent()

        def on_set_idle_state(state):
            if state:
                enabled.set()
            else:
                disabled.set()

        self.system.transmitter.on_set_idle_state = on_set_idle_state
        self.startup()

        self.system.obc.comm_set_idle_state(True)
        self.assertTrue(enabled.wait_for_change(1))

        self.system.obc.comm_set_idle_state(False)
        self.assertTrue(disabled.wait_for_change(1))
