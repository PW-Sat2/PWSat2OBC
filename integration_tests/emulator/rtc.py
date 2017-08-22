import telecommand
from datetime import timedelta
import time
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind
from bitarray import bitarray
from struct import pack

from beacon_parser.full_beacon_parser import FullBeaconParser

class RTCModule(ModuleBase):
    GridPos = (1, 0)

    def __init__(self, system):
        self._system = system
        self._rtc = system.rtc  # type: RTCDevice
        self.title = 'RTC'
        self.grid_pos = (1, 2)
        self.grid_span = (1, 1)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'RTCModule')

        self.bind_handlers()

        self._time = xrc.XRCCTRL(self._panel, 'rtc_time')

    def root(self):
        return self._panel

    def update(self):
        t = self._rtc.response_time()
        self._time.SetLabel('RTC time: \n' + t.strftime('%Y-%m-%d %H:%M:%S'))

    @bind('rtc_start', wx.EVT_BUTTON)
    def _on_start(self, evt):
        self._rtc.start_running()

    @bind('rtc_stop', wx.EVT_BUTTON)
    def _on_stop(self, evt):
        self._rtc.stop_running()

    @bind('rtc_advance_5min', wx.EVT_BUTTON, args=(timedelta(minutes=5),))
    def _on_advance(self, evt, interval):
        self._rtc.advance_by(interval)


