from datetime import timedelta, datetime
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind


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

    @bind('rtc_system_time', wx.EVT_BUTTON,)
    def _on_use_system_time(self, evt):
        self._rtc._current_time = datetime.now()

    @bind('rtc_advance_value', wx.EVT_TEXT)
    def _on_advance_time_value_changed(self, evt):
        new_text = evt.EventObject.GetValue()
        new_value = 1000
        try:
            new_value = int(new_text)
        except:
            pass
        self._rtc._advance_time_interval = timedelta(milliseconds=new_value)
