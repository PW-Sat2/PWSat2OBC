import os


class Emulator:
    def __init__(self, modules):
        self._modules = modules

    def start(self, wx_app):
        import wx
        from wx import xrc

        res = xrc.XmlResource(os.path.join(os.path.dirname(__file__), 'gui.xrc'))

        self._frame = res.LoadFrame(None, 'EmptyFrame')  # type: wx.Frame
        self._frame.SetDoubleBuffered(True)

        modules_sizer = self._frame.GetSizer()

        for module in self._modules:
            module_sizer = wx.StaticBoxSizer(wx.StaticBox(self._frame, label=module.title))
            modules_sizer.Add(module_sizer, module.grid_pos, span=module.grid_span)
            module_panel = wx.Panel(self._frame)
            module_sizer.Add(module_panel)
            module.load(res, module_panel)

            module.root().Layout()

        modules_sizer.Fit(self._frame)

        self._frame.Show()
        self._frame.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

        self._frame.Icon = wx.Icon(os.path.join(os.path.dirname(__file__), 'icon.ico'), wx.BITMAP_TYPE_ICO)

        self._update_timer = wx.Timer(wx_app, 100)
        wx.EVT_TIMER(wx_app, self._update_timer.GetId(), self._on_update)
        self._update_timer.Start(250)

    def OnCloseWindow(self, event):
        event.Veto(True)
        self._frame.Iconize(True)



    def _on_update(self, evt):
        for module in self._modules:
            module.update()


