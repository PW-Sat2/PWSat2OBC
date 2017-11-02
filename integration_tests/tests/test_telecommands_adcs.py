from response_frames.adcs import *
from system import auto_power_on
from telecommand import SetBuiltinDetumblingBlockMaskTelecommand, SetAdcsModeTelecommand, AdcsMode
from tests.base import RestartPerTest
from utils import TestEvent

class TestAdcsTelecommands(RestartPerTest):
    @auto_power_on(auto_power_on=True)
    def __init__(self, *args, **kwargs):
        super(TestAdcsTelecommands, self).__init__(*args, **kwargs)

    def test_set_adcs_mode_success(self):

        self.system.comm.put_frame(SetAdcsModeTelecommand(10, AdcsMode.ExperimentalSunpointing))

        response = self.system.comm.get_frame(5, filter_type=SetAdcsModeSuccessFrame)
        self.assertIsInstance(response, SetAdcsModeSuccessFrame)
        self.assertEqual(response.seq(), 0)
        self.assertEqual(response.correlation_id, 10)
        self.assertEqual(self.system.obc.adcs_current(), AdcsMode.ExperimentalSunpointing)

    def test_set_adcs_mode_failure(self):

        self.system.comm.put_frame(SetAdcsModeTelecommand(11, 23))

        response = self.system.comm.get_frame(5, filter_type=SetAdcsModeErrorFrame)
        self.assertIsInstance(response, SetAdcsModeErrorFrame)
        self.assertEqual(response.seq(), 0)
        self.assertEqual(response.correlation_id, 11)

    def test_set_builtin_detumbling_mask(self):

        self.system.comm.put_frame(SetBuiltinDetumblingBlockMaskTelecommand(10, True))

        response = self.system.comm.get_frame(5, filter_type=SetInternalDetumblingModeSuccessFrame)
        self.assertIsInstance(response, SetInternalDetumblingModeSuccessFrame)
        self.assertEqual(response.seq(), 0)
        self.assertEqual(response.correlation_id, 10)
        self.assertEqual(self.system.obc.state_get_adcs_config(), '1\nOK')
