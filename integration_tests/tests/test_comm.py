import struct

from build_config import config
from tests.base import BaseTest
from system import auto_comm_handling, auto_power_on
from utils import TestEvent
from obc import *
from devices import *

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

        self.assertEqual(frame[4:], "ABC")

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
        frame = "".join([chr(ord('A') + i % 25) for i in xrange(0, Comm.MAX_UPLINK_CONTENT_SIZE)])
        self.system.receiver.put_frame(frame)

        received_frame = self.system.obc.receive_frame()

        self.assertEqual(received_frame[4:], frame)

    def test_build_receive_frame_response(self):
        self.power_on_obc()
        data = "a" * 300
        doppler = 412
        rssi = 374

        response = ReceiverDevice.build_frame_response(data, doppler, rssi)

        self.assertEqual(response[0:2], [0x30, 0x01], "Length")
        self.assertEqual(response[2:4], [0x9C, 0x01], "Doppler")
        self.assertEqual(response[4:6], [0x76, 0x01], "RSSI")
        self.assertEqual(response[10:311], [ord('a')] * 300)

    def test_auto_pingpong(self):
        def reset_handler(*args):
            return False
        
        self.system.comm.on_hardware_reset = reset_handler
        self.system.comm.receiver.on_reset = reset_handler
        self.power_on_and_wait()

        frame = 'PING'

        self.system.receiver.put_frame(frame)
        msg = self.system.transmitter.get_message_from_buffer(20)

        msg = ''.join([chr(c) for c in msg])

        self.assertEqual(msg, "PONG")

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
