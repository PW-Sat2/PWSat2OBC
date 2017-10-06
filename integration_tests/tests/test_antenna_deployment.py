from datetime import timedelta

from system import runlevel, clear_state
from tests.base import RestartPerTest
from utils import TestEvent


class Test_AntennaDeployment(RestartPerTest):
    def __init__(self, methodName='runTest'):
        super(Test_AntennaDeployment, self).__init__(methodName)
        self.longMessage = True

    @runlevel(1)
    @clear_state()
    def test_antennas_are_not_deployed_too_soon(self):
        event = TestEvent()
        self.system.primary_antenna.on_begin_deployment = event.set

        t = timedelta(minutes=20)

        for i in xrange(0, 10):
            t += timedelta(seconds=60)

            self.system.obc.jump_to_time(t)
            self.system.obc.run_mission()

        self.assertFalse(event.wait_for_change(1), "antenna deployment process began too soon")

    @runlevel(1)
    @clear_state()
    def test_full_procedure_is_performed(self):
        self.system.i2c.enable_bus_devices([self.system.primary_antenna.address], False)
        self.system.i2c.enable_pld_devices([self.system.backup_antenna.address], False)

        def control_antenna_power(*args, **kwargs):
            main = self.system.eps.ANTenna.is_on
            red = self.system.eps.ANTennaRed.is_on
            power = main or red
            self.system.i2c.enable_bus_devices([self.system.primary_antenna.address], power)
            self.system.i2c.enable_pld_devices([self.system.backup_antenna.address], power)

        self.system.eps.ANTenna.on_enable = control_antenna_power
        self.system.eps.ANTenna.on_disable = control_antenna_power
        self.system.eps.ANTennaRed.on_enable = control_antenna_power
        self.system.eps.ANTennaRed.on_disable = control_antenna_power

        arm_counter = [0, 0]
        disarm_counter = [0, 0]
        reset_counter = [0, 0]

        primary_deploy = [False, False, False, False, False]
        backup_deploy = [False, False, False, False, False]

        def on_primary_arm_change(state):
            if state:
                arm_counter[0] += 1
            else:
                disarm_counter[0] += 1

        def on_backup_arm_change(state):
            if state:
                arm_counter[1] += 1
            else:
                disarm_counter[1] += 1

        def on_primary_deploy(controller, antenna_id):
            if antenna_id == -1:
                primary_deploy[0] = True
            else:
                primary_deploy[antenna_id] = True

            return None

        def on_backup_deploy(controller, antenna_id):
            if antenna_id == -1:
                backup_deploy[0] = True
            else:
                backup_deploy[antenna_id] = True

            return None

        def on_primary_reset():
            reset_counter[0] += 1

        def on_backup_reset():
            reset_counter[1] += 1

        self.system.primary_antenna.on_arm_state_change = on_primary_arm_change
        self.system.backup_antenna.on_arm_state_change = on_backup_arm_change

        self.system.primary_antenna.on_begin_deployment = on_primary_deploy
        self.system.backup_antenna.on_begin_deployment = on_backup_deploy

        self.system.primary_antenna.on_reset = on_primary_reset
        self.system.backup_antenna.on_reset = on_backup_reset

        t = timedelta(minutes=41)

        for i in xrange(0, 100):
            t += timedelta(seconds=60)

            self.system.obc.jump_to_time(t)
            self.system.obc.run_mission()

        self.assertEqual(arm_counter, [5, 5], "Controllers should be armed 5 times")
        self.assertEqual(disarm_counter, [5, 5], "Controllers should be disarmed 5 times")

        self.assertEqual(primary_deploy, [True] * 5, "All antennas should be deployed by primary controller")
        self.assertEqual(backup_deploy, [True] * 5, "All antennas should be deployed by backup controller")

        self.assertEqual(reset_counter, [5, 5], "Controllers are reseted 5 times")
