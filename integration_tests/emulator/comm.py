import telecommand
from datetime import timedelta
import time
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind
from bitarray import bitarray
from struct import pack
from beacon_parser.parser import BitReader, BeaconStorage

from beacon_parser.full_beacon_parser import FullBeaconParser

class CommModule(ModuleBase):
    GridPos = (1, 0)

    def __init__(self, system):
        self._system = system
        self.title = 'Comm'
        self.grid_pos = (1, 3)
        self.grid_span = (1, 1)
        self._last_beacon_seen = None
        self._last_mission_time_seen = 0

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'COMMModule')

        self.bind_handlers()

        self._current_comm_queue_size = xrc.XRCCTRL(self._panel, 'current_comm_queue_size')

        self._current_beacon_timestamp = xrc.XRCCTRL(self._panel, 'current_beacon_timestamp')

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

        if self._system.transmitter.current_beacon_timestamp is not None:
            self._current_beacon_timestamp.SetLabel(
                'Current beacon: \n' + time.strftime('%Y-%m-%d %H:%M:%S', self._system.transmitter.current_beacon_timestamp)
                + ' (' + str(len(self._system.transmitter.current_beacon)) + ' bytes)')
