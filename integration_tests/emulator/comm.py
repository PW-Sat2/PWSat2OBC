import telecommand
import time
import wx
from wx import xrc
from system import System
from .base import ModuleBase, bind
import Queue


class CommModule(ModuleBase):
    GridPos = (1, 0)

    def __init__(self, system, last_beacon):
        self._system = system # type: System
        self._last_beacon = last_beacon
        self.title = 'Comm'
        self.grid_pos = (1, 3)
        self.grid_span = (1, 1)
        self._last_beacon_seen = None
        self._last_mission_time_seen = 0
        self._last_message_processing_start = None

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'COMMModule')

        self.bind_handlers()

        self._current_comm_queue_size = xrc.XRCCTRL(self._panel, 'current_comm_queue_size')
        self._current_beacon_timestamp = xrc.XRCCTRL(self._panel, 'current_beacon_timestamp')

        self._transmitter_last_watchdog = xrc.XRCCTRL(self._panel, 'last_watchdog_transmitter')
        self._receiver_last_watchdog = xrc.XRCCTRL(self._panel, 'last_watchdog_receiver')
        self._is_transmitter_simulated_checkbox = xrc.XRCCTRL(self._panel, 'is_transmitter_simulated')

    def root(self):
        return self._panel

    @bind('send_beacon', wx.EVT_BUTTON)
    def _send_beacon(self, evt):
        self._system.comm.put_frame(telecommand.SendBeacon())
        beacon_frame = self._system.comm.get_frame(30)
        self._system.transmitter.current_beacon_timestamp = time.localtime()
        self._system.transmitter.current_beacon = beacon_frame.payload()

    @bind('comm_reset_queue', wx.EVT_BUTTON)
    def _reset_queue(self, evt):
        self._system.transmitter.reset()

    @bind('comm_get_frame', wx.EVT_BUTTON)
    def _get_frame(self, evt):
        self._system.comm.get_frame(20)

    def update(self):
        self._current_comm_queue_size.SetLabel('Comm queue size: ' + str(self._system.transmitter.queue_size()))

        if time.mktime(time.localtime()) - self._last_mission_time_seen > 4:
            self._last_mission_time_seen = time.mktime(time.localtime())

        if self._last_beacon.time is not None:
            self._current_beacon_timestamp.SetLabel(
                'Current beacon: \n' + self._last_beacon.time.strftime('%Y-%m-%d %H:%M:%S')
                + ' (' + str(len(self._last_beacon.payload.payload())) + ' bytes)')

        if self._system.comm.receiver.last_watchdog_kick is None:
            self._receiver_last_watchdog.SetLabel('Receiver: last watchdog kick\nNone')
        else:
            self._receiver_last_watchdog.SetLabel('Receiver: last watchdog kick\n{:%Y-%m-%d %H:%M:%S}'
                                                  .format(self._system.comm.receiver.last_watchdog_kick))

        if self._system.comm.transmitter.last_watchdog_kick is None:
            self._transmitter_last_watchdog.SetLabel('Transmitter: last watchdog kick\nNone')
        else:
            self._transmitter_last_watchdog.SetLabel('Transmitter: last watchdog kick\n{:%Y-%m-%d %H:%M:%S}'
                                                  .format(self._system.comm.transmitter.last_watchdog_kick))

        self.transmitter_simulator_loop()

    def transmitter_simulator_loop(self):
        if not self._is_transmitter_simulated_checkbox.GetValue():
            return

        # if there is no frame in queue and no frame is "still processed" -> Skip
        if self._system.transmitter.queue_size() < 1 and not self._last_message_processing_start:
            return

        if not self._last_message_processing_start:
            # Start processing of new frame
            self._last_message_processing_start = time.time()
            try:
                self._system.comm.get_frame(0.01)
            except Queue.Empty:
                pass
        else:
            # Simulate period of processing with rough estimate of sending time.
            # Frame length in bits divided by baud rate plus one second for simulate delays
            message_sending_time = 8.0 * 235.0 / float(str(self._system.transmitter.baud_rate)) + 1.0
            if time.time() - self._last_message_processing_start < message_sending_time:
                return

            # finish processing
            self._last_message_processing_start = None
