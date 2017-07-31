from response_frames.fdir import ErrorCountersFrame
from response_frames.operation import OperationSuccessFrame
from system import auto_power_on, clear_state
from telecommand import SetErrorCounterConfig, GetErrorCounterConfig
from tests.base import RestartPerTest
from utils import TestEvent


class TestFDIRTelecommands(RestartPerTest):
    @auto_power_on(False)
    def __init__(self, *args, **kwargs):
        super(TestFDIRTelecommands, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @clear_state()
    def test_should_set_error_counters_config(self):
        self._start()
        # assert: default config
        current_config = self.system.obc.error_counters()

        self.assertEqual(current_config[0].limit, 128)
        self.assertEqual(current_config[0].increment, 5)
        self.assertEqual(current_config[0].decrement, 2)

        self.assertEqual(current_config[2].limit, 128)
        self.assertEqual(current_config[2].increment, 5)
        self.assertEqual(current_config[2].decrement, 2)

        device_0 = (0, 137, 7, 3)
        device_1 = (2, 96, 0, 4)

        self.system.comm.put_frame(SetErrorCounterConfig(0x21, [device_0, device_1]))

        response = self.system.comm.get_frame(5)

        self.assertIsInstance(response, OperationSuccessFrame)
        self.assertEqual(response.correlation_id, 0x21)
        self.assertEqual(response.response, [0, 2])

        current_config = self.system.obc.error_counters()

        self.assertEqual(current_config[0].limit, 137)
        self.assertEqual(current_config[0].increment, 7)
        self.assertEqual(current_config[0].decrement, 3)

        self.assertEqual(current_config[2].limit, 96)
        self.assertEqual(current_config[2].increment, 0)
        self.assertEqual(current_config[2].decrement, 4)

    def test_should_get_error_counters_config(self):
        self._start()

        current_config = self.system.obc.error_counters()

        self.system.comm.put_frame(GetErrorCounterConfig())

        f = self.system.comm.get_frame(5)

        self.assertIsInstance(f, ErrorCountersFrame)

        for i in xrange(0, 12):
            self.assertEqual(current_config[i].current, f.counters[i]['current'], "Device {}".format(i))
            self.assertEqual(current_config[i].limit, f.counters[i]['limit'], "Device {}".format(i))
            self.assertEqual(current_config[i].increment, f.counters[i]['increment'], "Device {}".format(i))
            self.assertEqual(current_config[i].decrement, f.counters[i]['decrement'], "Device {}".format(i))
