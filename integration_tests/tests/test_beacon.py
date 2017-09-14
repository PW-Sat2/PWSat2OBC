from datetime import timedelta, datetime

import logging
from tests.base import RestartPerTest
from utils import TestEvent
from system import auto_power_on, runlevel


class Test_Beacon(RestartPerTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(Test_Beacon, self).__init__(methodName)

    def begin_deployment(self):
        self.system.obc.jump_to_time(timedelta(minutes=43))

    def next_step(self):
        self.system.obc.run_mission()

    def run_steps(self, count):
        while count > 0:
            self.next_step()
            count -= 1

    def begin(self, count=1):
        self.power_on_obc()
        self.begin_deployment()
        self.run_steps(count)

    @runlevel(1)
    def test_beacon_auto_activation(self):
        event = TestEvent()

        def reset_handler(*args):
            return False

        self.system.primary_antenna.begin_deployment()
        self.system.primary_antenna.finish_deployment()
        self.system.backup_antenna.begin_deployment()
        self.system.backup_antenna.finish_deployment()
        self.system.primary_antenna.on_reset = reset_handler
        self.system.backup_antenna.on_reset = reset_handler
        self.system.comm.transmitter.on_set_beacon = event.set
        self.begin(19)
        self.assertTrue(event.wait_for_change(1), "beacon should be set once the antennas are deployed")


class Test_Beacon_Restarting(RestartPerTest):
    @runlevel(1)
    def test_beacon_on_backward_time_correction(self):
        event = TestEvent()


        self.system.comm.transmitter.on_set_beacon = event.set

        log = logging.getLogger("test_beacon")



        # Set initial state: deployed antennas, T=+60m, and run mission for save everyting into persistent state
        self.system.obc.state_set_antenna(1)
        start_time = datetime.now()
        self.system.rtc.set_response_time(start_time)
        self.system.obc.jump_to_time(timedelta(minutes=60))
        obc_start = self.system.obc.current_time()
        self.system.obc.run_mission()

        # Advance time just before time correction and expect beacon here
        self.system.obc.advance_time(timedelta(minutes=14,seconds=45))
        event.reset()
        self.system.comm.transmitter.reset()
        self.system.obc.run_mission()
        self.assertTrue(event.wait_for_change(1), "Beacon should be sent")

        # Perform negative time correction and ensure no beacon is set during that mission loop
        time_before_correction = self.system.obc.current_time()
        self.system.obc.advance_time(timedelta(seconds=16))
        self.system.rtc.set_response_time(start_time + timedelta(minutes=1))
        event.reset()
        self.system.comm.transmitter.reset()
        self.system.obc.run_mission()
        self.assertFalse(event.wait_for_change(0), "Beacon should not be sent")
        time_after_correction = self.system.obc.current_time()
        self.assertGreater(time_before_correction, time_after_correction, "Time should been corrected")

        # 3 subsequent beacons should be set
        for i in range(0,3):
            self.system.obc.advance_time(timedelta(seconds=32))
            event.reset()
            self.system.comm.transmitter.reset()
            self.system.obc.run_mission()
            self.assertTrue(event.wait_for_change(1), "Beacon should be sent %d" % i)
