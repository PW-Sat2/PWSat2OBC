import logging

import IPython.lib.guisupport
import wx
import wx.lib.newevent

wxLogEvent, EVT_WX_LOG_EVENT = wx.lib.newevent.NewEvent()


class LogViewer(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, wx.ID_ANY, "Log viewer", size=(700, 500), style=wx.CAPTION)

        # Add a panel so it looks the correct on all platforms
        panel = wx.Panel(self, wx.ID_ANY)
        self.log = wx.TextCtrl(panel, wx.ID_ANY, size=(150, 300),
                               style=wx.TE_MULTILINE | wx.TE_READONLY | wx.HSCROLL)

        self.log.Font = wx.Font(family=wx.FONTFAMILY_MODERN, pointSize=8, style=wx.FONTSTYLE_NORMAL,
                                weight=wx.FONTWEIGHT_NORMAL)

        # Add widgets to a sizer
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.log, 1, wx.ALL | wx.EXPAND, 5)
        panel.SetSizer(sizer)

        self.Bind(EVT_WX_LOG_EVENT, self.write_line)

    def write_line(self, evt):
        self.log.AppendText(evt.message + "\n")


class GuiLogger(logging.Handler):
    _form = None

    def use_form(self, form):
        self._form = form
        self._form.Bind(wx.EVT_WINDOW_DESTROY, self.on_destroy)

        return self

    def emit(self, record):
        msg = self.format(record)

        evt = wxLogEvent(message=msg, levelname=record.levelname)

        wx.PostEvent(self._form, evt)

    def on_destroy(self, _):
        logging.root.removeHandler(self)


app = IPython.lib.guisupport.get_app_wx()

form = LogViewer()

form.Show()

handler = GuiLogger().use_form(form)

formatter = logging.Formatter("%(asctime)-15s %(levelname)s: [%(name)s] %(message)s")

handler.setFormatter(formatter)

logging.root.setLevel(logging.DEBUG)
logging.root.handlers[:] = []
logging.root.addHandler(handler)
