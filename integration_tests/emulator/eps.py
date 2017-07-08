import wx
from wx import xrc

from .base import ModuleBase, bind

class EPSModule(ModuleBase):
    def __init__(self, system):
        self._system = system

        self.title = 'EPS'
        self.grid_pos = (1, 1)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'EPSModule')
        self.bind_handlers()

        self._tkmain_value = xrc.XRCCTRL(self._panel, 'eps_tkmain_value')
        self._tkred_value = xrc.XRCCTRL(self._panel, 'eps_tkred_value')

        self._eps_suns_value = xrc.XRCCTRL(self._panel, 'eps_suns_value')
        self._eps_camnadir_value = xrc.XRCCTRL(self._panel, 'eps_camnadir_value')
        self._eps_camwing_value = xrc.XRCCTRL(self._panel, 'eps_camwing_value')
        self._eps_sens_value = xrc.XRCCTRL(self._panel, 'eps_sens_value')
        self._eps_antenna_value = xrc.XRCCTRL(self._panel, 'eps_antenna_value')
        self._eps_antennared_value = xrc.XRCCTRL(self._panel, 'eps_antennared_value')

        self._eps_sailmain_value = xrc.XRCCTRL(self._panel, 'eps_sailmain_value')
        self._eps_sailred_value = xrc.XRCCTRL(self._panel, 'eps_sailred_value')
        self._eps_sadsmain_value = xrc.XRCCTRL(self._panel, 'eps_sadsmain_value')
        self._eps_sadsred_value = xrc.XRCCTRL(self._panel, 'eps_sadsred_value')

    def get_color(self, value):
        return '#009400' if value else 'red'

    def update(self):
        self._tkmain_value.SetLabel(str(self._system.eps.TKmain.is_on))
        self._tkred_value.SetLabel(str(self._system.eps.TKred.is_on))

        self._eps_suns_value.SetLabel(str(self._system.eps.SunS.is_on))
        self._eps_camnadir_value.SetLabel(str(self._system.eps.CamNadir.is_on))
        self._eps_camwing_value.SetLabel(str(self._system.eps.CamWing.is_on))
        self._eps_sens_value.SetLabel(str(self._system.eps.SENS.is_on))
        self._eps_antenna_value.SetLabel(str(self._system.eps.ANTenna.is_on))
        self._eps_antennared_value.SetLabel(str(self._system.eps.ANTennaRed.is_on))

        self._eps_sailmain_value.SetLabel(str(self._system.eps.SAILmain.enabled))
        self._eps_sailred_value.SetLabel(str(self._system.eps.SAILred.enabled))
        self._eps_sadsmain_value.SetLabel(str(self._system.eps.SADSmain.enabled))
        self._eps_sadsred_value.SetLabel(str(self._system.eps.SADSred.enabled))

        self._tkmain_value.SetForegroundColour(self.get_color(self._system.eps.TKmain.is_on))
        self._tkred_value.SetForegroundColour(self.get_color(self._system.eps.TKred.is_on))

        self._eps_suns_value.SetForegroundColour(self.get_color(self._system.eps.SunS.is_on))
        self._eps_camnadir_value.SetForegroundColour(self.get_color(self._system.eps.CamNadir.is_on))
        self._eps_camwing_value.SetForegroundColour(self.get_color(self._system.eps.CamWing.is_on))
        self._eps_sens_value.SetForegroundColour(self.get_color(self._system.eps.SENS.is_on))
        self._eps_antenna_value.SetForegroundColour(self.get_color(self._system.eps.ANTenna.is_on))
        self._eps_antennared_value.SetForegroundColour(self.get_color(self._system.eps.ANTennaRed.is_on))

        self._eps_sailmain_value.SetForegroundColour(self.get_color(self._system.eps.SAILmain.enabled))
        self._eps_sailred_value.SetForegroundColour(self.get_color(self._system.eps.SAILred.enabled))
        self._eps_sadsmain_value.SetForegroundColour(self.get_color(self._system.eps.SADSmain.enabled))
        self._eps_sadsred_value.SetForegroundColour(self.get_color(self._system.eps.SADSred.enabled))

    def root(self):
        return self._panel

    @bind('eps_tkmain_disable', wx.EVT_BUTTON, args=('TKmain',))
    @bind('eps_tkred_disable', wx.EVT_BUTTON, args=('TKred',))
    @bind('eps_suns_disable', wx.EVT_BUTTON, args=('SunS',))
    @bind('eps_camnadir_disable', wx.EVT_BUTTON, args=('CamNadir',))
    @bind('eps_camwing_disable', wx.EVT_BUTTON, args=('CamWing',))
    @bind('eps_sens_disable', wx.EVT_BUTTON, args=('SENS',))
    @bind('eps_antenna_disable', wx.EVT_BUTTON, args=('ANTenna',))
    @bind('eps_antennared_disable', wx.EVT_BUTTON, args=('ANTennaRed',))
    def _disable_lcl(self, evt, lcl_name):
        getattr(self._system.eps, lcl_name).off()
