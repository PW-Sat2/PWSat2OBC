from nose.tools import nottest

from devices import *
from obc import *
from response_frames.pong import PongFrame
from tests.base import BaseTest, RestartPerTest
from system import auto_power_on, runlevel
from utils import TestEvent, ensure_byte_list


class Test_Comm(RestartPerTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(Test_Comm, self).__init__(methodName)

        self.longMessage = True

    def module_reset(self, module, event):
        self.power_on_obc()
        self.system.obc.comm_reset(module)
        self.assertTrue(event.wait_for_change(1))

    @runlevel(2)
    def test_should_initialize_hardware(self):
        event = TestEvent()
        self.system.comm.on_hardware_reset = event.set
        self.power_on_obc()
        self.assertTrue(event.wait_for_change(1))

    @runlevel(1)
    def test_transmitter_reset(self):
        event = TestEvent()
        self.system.comm.transmitter.on_reset = event.set
        self.module_reset(CommModule.Transmitter, event)

    @runlevel(1)
    def test_receiver_reset(self):
        event = TestEvent()
        self.system.comm.receiver.on_reset = event.set
        self.module_reset(CommModule.Receiver, event)

    @runlevel(1)
    def test_hardware_reset(self):
        event = TestEvent()
        self.system.comm.on_hardware_reset = event.set
        self.module_reset(CommModule.Both, event)

    @runlevel(1)
    def test_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.module_reset(CommModule.Watchdog, event)

    @runlevel(1)
    def test_should_send_frame(self):
        self.power_on_obc()
        self.system.obc.send_frame("ABC")
        msg = self.system.transmitter.get_message_from_buffer(3)
        self.assertEqual(msg, (65, 66, 67))

    @runlevel(1)
    def test_should_get_number_of_frames(self):
        self.power_on_obc()
        self.system.receiver.put_frame("ABC")

        count = int(self.system.obc.get_frame_count())

        self.assertEqual(count, 1)

    @runlevel(1)
    def test_should_receive_frame(self):
        self.power_on_obc()
        self.system.receiver.put_frame("ABC")

        frame = self.system.obc.receive_frame()

        self.assertEqual(frame[0:], "ABC")

    @runlevel(1)
    def test_should_remove_frame_after_receive(self):
        event = TestEvent()
        self.system.receiver.on_frame_remove = event.set
        self.power_on_obc()
        self.system.receiver.put_frame("ABD")

        self.system.obc.receive_frame()
        self.assertTrue(event.wait_for_change(1))

        self.assertEqual(self.system.receiver.queue_size(), 0)

    @runlevel(2)
    def test_auto_pingpong(self):
        def reset_handler(*args):
            return False

        self.system.comm.on_hardware_reset = reset_handler
        self.system.comm.receiver.on_reset = reset_handler
        self.power_on_obc()

        self.system.comm.put_frame(UplinkFrame(ord('P'), 'ABC'))

        msg = self.system.comm.get_frame(20, filter_type=PongFrame)
        self.assertIsInstance(msg, PongFrame)
        self.assertEqual(msg.payload(), ensure_byte_list("PONG"))

    @runlevel(2)
    def test_auto_watchdog_reset(self):
        event = TestEvent()
        self.system.comm.on_watchdog_reset = event.set
        self.power_on_obc()
        self.assertTrue(event.wait_for_change(5))

    @runlevel(1)
    def test_set_baud_rate(self):
        event = TestEvent()
        self.system.transmitter.on_set_baudrate = event.set
        self.power_on_obc()
        self.system.obc.comm_set_bitrate(BaudRate.BaudRate9600)
        event.wait_for_change(2)
        self.assertEqual(self.system.transmitter.baud_rate, BaudRate.BaudRate9600)

    @runlevel(1)
    def test_get_transmitter_telemetry(self):
        self.system.transmitter.on_report_uptime = lambda *args: [1, 2, 3, 4]
        self.system.transmitter.on_report_state = lambda *args: [0b00001001]
        self.system.transmitter.on_get_telemetry_last_transmission = \
            lambda *args: TransmitterTelemetry.build(1444, 1666, 1222, 1555)

        self.system.transmitter.on_get_telemetry_instant = \
            lambda *args: TransmitterTelemetry.build(1234, 1234, 1111, 1333)

        self.power_on_obc()
        telemetry = self.system.obc.comm_get_transmitter_telemetry()
        expected = {
            'Uptime': 356521,
            'Now RF Forward power': 1111,
            'LastTransmitted RF Forward power': 1222,
            'Now TX Current': 1333,
            'LastTransmitted RF Reflected power': 1444,
            'Beacon': 0,
            'Idle state': 1,
            'LastTransmitted TX Current': 1555,
            'Bitrate': 4,
            'LastTransmitted Power Amp Temperature': 1666
        }

        for k in expected.keys():
            self.assertEqual(expected[k], telemetry[k], "Key: %s" % k)

    @runlevel(1)
    def test_get_receiver_telemetry_no_frame_received(self):
        self.system.receiver.on_report_uptime = lambda *args: [4, 3, 2, 1]
        self.system.receiver.on_get_telemetry = \
            lambda *args: ReceiverTelemetry.build(12, 1110, 1929, 2748, 3567, 564, 1383)

        self.power_on_obc()
        telemetry = self.system.obc.comm_get_receiver_telemetry()

        expected = {
            'Uptime': 93784,
            'Now Oscillator Temperature': 3567,
            'Now Power Amp Temperature': 564,
            'Now RX current': 1110,
            'LastReceived RSSI': 0,
            'Now Power Supply Voltage': 2748,
            'LastReceived Doppler': 0,
            'Now Doppler': 1929,
            'Now RSSI': 1383
        }

        for k in expected.keys():
            self.assertEqual(expected[k], telemetry[k], "Key: %s" % k)

    @runlevel(1)
    def test_get_receiver_telemetry_frame_received(self):
        self.system.receiver.on_report_uptime = lambda *args: [4, 3, 2, 1]
        self.system.receiver.on_get_telemetry = \
            lambda *args: ReceiverTelemetry.build(12, 1110, 1929, 2748, 3567, 564, 1383)

        self.power_on_obc()

        self.system.comm.receiver.put_frame('ABCD')

        self.system.obc.receive_frame()

        telemetry = self.system.obc.comm_get_receiver_telemetry()

        expected = {
            'Uptime': 93784,
            'Now Oscillator Temperature': 3567,
            'Now Power Amp Temperature': 564,
            'Now RX current': 1110,
            'LastReceived RSSI': 300,
            'Now Power Supply Voltage': 2748,
            'LastReceived Doppler': 257,
            'Now Doppler': 1929,
            'Now RSSI': 1383
        }

        for k in expected.keys():
            self.assertEqual(expected[k], telemetry[k], "Key: %s" % k)

    @runlevel(1)
    def test_set_idle_state(self):
        enabled = TestEvent()
        disabled = TestEvent()

        def on_set_idle_state(state):
            if state:
                enabled.set()
            else:
                disabled.set()

        self.system.transmitter.on_set_idle_state = on_set_idle_state
        self.power_on_obc()

        self.system.obc.comm_set_idle_state(True)
        self.assertTrue(enabled.wait_for_change(1))

        self.system.obc.comm_set_idle_state(False)
        self.assertTrue(disabled.wait_for_change(1))
