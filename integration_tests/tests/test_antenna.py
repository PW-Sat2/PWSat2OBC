from tests.base import BaseTest
from obc import *
from devices import *
from utils import TestEvent
from system import auto_power_on

class Test_Antenna(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName = 'runTest'):
        super(Test_Antenna, self).__init__(methodName)

    def test_primary_antenna_is_reset_at_startup(self):
        event = TestEvent()
        self.system.primary_antenna.on_reset = event.set
        self.power_on_and_wait()
        self.assertTrue(event.wait_for_change(1))

    def test_backup_antenna_is_reset_at_startup(self):
        event = TestEvent()
        self.system.backup_antenna.on_reset = event.set
        self.power_on_and_wait()
        self.assertTrue(event.wait_for_change(1))

    def test_auto_deployment(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == -1:
                event.set()

        self.system.primary_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        self.system.obc.antenna_deploy(AntennaChannel.Primary, AntennaId.Auto, OverrideSwitches.Disabled)
        self.assertTrue(event.wait_for_change(1))

    def test_manual_deployment(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == 2:
                event.set()

        self.system.primary_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        self.system.obc.antenna_deploy(AntennaChannel.Primary, AntennaId.Antenna2, OverrideSwitches.Disabled)
        self.assertTrue(event.wait_for_change(1))

    def test_manual_deployment_with_override(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == 2:
                event.set()

        self.system.backup_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        result = self.system.obc.antenna_deploy(AntennaChannel.Backup, AntennaId.Antenna2, OverrideSwitches.Enabled)
        self.assertTrue(event.wait_for_change(1))
        self.assertTrue(self.system.backup_antenna.ignore_deployment_switch)

    def test_deployment_finalization(self):
        event = TestEvent()
        self.system.backup_antenna.on_deployment_cancel = event.set
        self.power_on_and_wait()
        result = self.system.obc.antenna_cancel_deployment(AntennaChannel.Backup)
        self.assertTrue(event.wait_for_change(1))

    def test_telemetry(self):
        def return_state():
            return [0x0f, 0xe0]

        self.system.backup_antenna.on_get_deployment_status = return_state
        self.power_on_and_wait()
        result = self.system.obc.antenna_get_status(AntennaChannel.Backup)
        self.assertTrue(result.Status)
        self.assertTrue(result.SystemArmed)
        self.assertFalse(result.IgnoringSwitches)

        self.assertFalse(result.DeploymentState[0], "Antenna 1 should not be deployed")
        self.assertTrue(result.DeploymentState[1], "Antenna 2 should be deployed")
        self.assertTrue(result.DeploymentState[2], "Antenna 3 should be deployed")
        self.assertFalse(result.DeploymentState[3], "Antenna 4 should not be deployed")

        self.assertTrue(result.DeploymentInProgress[0], "Antenna 1 deployment process should be still be running")
        self.assertFalse(result.DeploymentInProgress[1], "Antenna 2 deployment process should not be still be running")
        self.assertFalse(result.DeploymentInProgress[2], "Antenna 3 deployment process should not be still be running")
        self.assertTrue(result.DeploymentInProgress[3], "Antenna 4 deployment process should be still be running")

    def test_get_telemetry(self):
        def reset_handler():
            return False

        def primary_temperature():
            return 0x1ff

        def backup_temperature():
            return 0x2ff

        self.system.primary_antenna.on_reset = reset_handler
        self.system.backup_antenna.on_reset = reset_handler
        self.system.primary_antenna.on_get_temperature = primary_temperature
        self.system.backup_antenna.on_get_temperature = backup_temperature
        self.system.primary_antenna.antenna_state = [
            Antenna.build(True, 0xb0, 0x8765, True),
            Antenna.build(False, 0xc0, 0x7654, True),
            Antenna.build(True, 0xd0, 0x6543, False),
            Antenna.build(False, 0xe0, 0x5432, False),
            ]

        self.power_on_and_wait()
        result = self.system.obc.antenna_get_telemetry()
        self.assertEqual(result.ActivationCount[0], 0xb0);
        self.assertEqual(result.ActivationCount[1], 0xc0);
        self.assertEqual(result.ActivationCount[2], 0xd0);
        self.assertEqual(result.ActivationCount[3], 0xe0);
        self.assertEqual(result.ActivationTime[0], 0x8765 * 50);
        self.assertEqual(result.ActivationTime[1], 0x7654 * 50);
        self.assertEqual(result.ActivationTime[2], 0x6543 * 50);
        self.assertEqual(result.ActivationTime[3], 0x5432 * 50);
        self.assertEqual(result.Temperature[0], 0x1ff);
        self.assertEqual(result.Temperature[1], 0x2ff);

