import wx
from wx import xrc

from .base import ModuleBase, bind
from devices.eps import LCL


class EPSModule(ModuleBase):
    def __init__(self, system):
        self._system = system

        self.title = 'EPS'
        self.grid_pos = (1, 0)
        self.grid_span = (1, 1)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'EPSModule')
        self.bind_handlers()

        self._tkmain = xrc.XRCCTRL(self._panel, 'tkmain')
        self._tkred = xrc.XRCCTRL(self._panel, 'tkred')

        self._eps_suns = xrc.XRCCTRL(self._panel, 'suns')
        self._eps_camnadir = xrc.XRCCTRL(self._panel, 'cam_nadir')
        self._eps_camwing = xrc.XRCCTRL(self._panel, 'cam_wing')
        self._eps_sens = xrc.XRCCTRL(self._panel, 'sens')
        self._eps_antenna = xrc.XRCCTRL(self._panel, 'antenna')
        self._eps_antennared = xrc.XRCCTRL(self._panel, 'antenna_red')

        self._eps_sailmain_value = xrc.XRCCTRL(self._panel, 'eps_sailmain_value')
        self._eps_sailred_value = xrc.XRCCTRL(self._panel, 'eps_sailred_value')
        self._eps_sadsmain_value = xrc.XRCCTRL(self._panel, 'eps_sadsmain_value')
        self._eps_sadsred_value = xrc.XRCCTRL(self._panel, 'eps_sadsred_value')

    def get_color(self, value):
        return '#009400' if value else 'red'

    def update(self):
        self._tkmain.SetValue(self._system.eps.TKmain.is_on)
        self._tkred.SetValue(self._system.eps.TKred.is_on)
        self._eps_suns.SetValue(self._system.eps.SunS.is_on)
        self._eps_camnadir.SetValue(self._system.eps.CamNadir.is_on)
        self._eps_camwing.SetValue(self._system.eps.CamWing.is_on)
        self._eps_sens.SetValue(self._system.eps.SENS.is_on)
        self._eps_antenna.SetValue(self._system.eps.ANTenna.is_on)
        self._eps_antennared.SetValue(self._system.eps.ANTennaRed.is_on)

        self._eps_sailmain_value.SetLabel(str(self._system.eps.SAILmain.enabled))
        self._eps_sailred_value.SetLabel(str(self._system.eps.SAILred.enabled))
        self._eps_sadsmain_value.SetLabel(str(self._system.eps.SADSmain.enabled))
        self._eps_sadsred_value.SetLabel(str(self._system.eps.SADSred.enabled))

        self._eps_sailmain_value.SetForegroundColour(self.get_color(self._system.eps.SAILmain.enabled))
        self._eps_sailred_value.SetForegroundColour(self.get_color(self._system.eps.SAILred.enabled))
        self._eps_sadsmain_value.SetForegroundColour(self.get_color(self._system.eps.SADSmain.enabled))
        self._eps_sadsred_value.SetForegroundColour(self.get_color(self._system.eps.SADSred.enabled))

    def root(self):
        return self._panel

    @bind('tkmain', wx.EVT_TOGGLEBUTTON, args=('TKmain',))
    @bind('tkred', wx.EVT_TOGGLEBUTTON, args=('TKred',))
    @bind('suns', wx.EVT_TOGGLEBUTTON, args=('SunS',))
    @bind('cam_nadir', wx.EVT_TOGGLEBUTTON, args=('CamNadir',))
    @bind('cam_wing', wx.EVT_TOGGLEBUTTON, args=('CamWing',))
    @bind('sens', wx.EVT_TOGGLEBUTTON, args=('SENS',))
    @bind('antenna', wx.EVT_TOGGLEBUTTON, args=('ANTenna',))
    @bind('antenna_red', wx.EVT_TOGGLEBUTTON, args=('ANTennaRed',))
    def toggle(self, evt, lcl_name):
        lcl = getattr(self._system.eps, lcl_name)  # type: LCL

        v = evt.EventObject.GetValue()

        if lcl.is_on and not v:
            lcl.off()
        elif not lcl.is_on and v:
            lcl.on()
