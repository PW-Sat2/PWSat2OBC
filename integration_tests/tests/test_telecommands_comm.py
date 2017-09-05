import telecommand
from struct import pack
from response_frames.operation import OperationSuccessFrame
from devices import BeaconFrame
from system import auto_power_on, runlevel
from tests.base import BaseTest, RestartPerTest
from utils import ensure_byte_list, TestEvent
from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BitReader, BeaconStorage, BitArrayParser


class CommTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(CommTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @runlevel(2)
    def test_enter_idle_state(self):
        event = TestEvent()

        def on_set_idle_state(state):
            if state:
                event.set()

        self.system.transmitter.on_set_idle_state = on_set_idle_state

        self._start()

        self.system.comm.put_frame(telecommand.EnterIdleState(correlation_id=0x11, duration=5))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)

        self.assertTrue(event.wait_for_change(1))

    @runlevel(2)
    def test_leave_idle_state(self):
        event = TestEvent()

        def on_set_idle_state(state):
            if not state:
                event.set()

        self.system.transmitter.on_set_idle_state = on_set_idle_state

        self._start()

        self.system.comm.put_frame(telecommand.EnterIdleState(correlation_id=0x11, duration=1))

        frame = self.system.comm.get_frame(20)
        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)

        self.assertTrue(event.wait_for_change(30))

    @runlevel(2)
    def test_send_beacon(self):
        self._start()

        self.system.comm.put_frame(telecommand.SendBeacon())

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, BeaconFrame)

    @runlevel(2)
    def test_beacon_parsing(self):
        self._start()

        self.system.comm.put_frame(telecommand.SendBeacon())

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, BeaconFrame)
        store = BeaconStorage()
        parser = BitArrayParser(FullBeaconParser(),
                                ''.join(map(lambda x: pack('B', x), frame._payload)),
                                store)
        parser.parse()
