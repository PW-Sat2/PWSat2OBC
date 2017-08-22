import wx
from wx import xrc

from .base import ModuleBase, bind


class PayloadModule(ModuleBase):
    def __init__(self, system):
        self._system = system

        self.title = 'Payload'
        self.grid_pos = (0, 3)
        self.grid_span = (1, 2)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'PayloadModule')
        self.bind_handlers()

        self._timeout_value = xrc.XRCCTRL(self._panel, 'pld_timeout_value')

        self._whoami_value = xrc.XRCCTRL(self._panel, 'pld_whoami_value')

        self._hk_int_value = xrc.XRCCTRL(self._panel, 'pld_hk_int_value')
        self._hk_obc_value = xrc.XRCCTRL(self._panel, 'pld_hk_obc_value')

        self._radfet_status_value = xrc.XRCCTRL(self._panel, 'pld_radfet_status_value')
        self._radfet_temp_value = xrc.XRCCTRL(self._panel, 'pld_radfet_temp_value')
        self._radfet_vth1_value = xrc.XRCCTRL(self._panel, 'pld_radfet_vth1_value')
        self._radfet_vth2_value = xrc.XRCCTRL(self._panel, 'pld_radfet_vth2_value')
        self._radfet_vth3_value = xrc.XRCCTRL(self._panel, 'pld_radfet_vth3_value')

        self._suns_v1_value = xrc.XRCCTRL(self._panel, 'pld_suns_v1_value')
        self._suns_v2_value = xrc.XRCCTRL(self._panel, 'pld_suns_v2_value')
        self._suns_v3_value = xrc.XRCCTRL(self._panel, 'pld_suns_v3_value')
        self._suns_v4_value = xrc.XRCCTRL(self._panel, 'pld_suns_v4_value')
        self._suns_v5_value = xrc.XRCCTRL(self._panel, 'pld_suns_v5_value')

        self._ph_xp_value = xrc.XRCCTRL(self._panel, 'pld_ph_xp_value')
        self._ph_xn_value = xrc.XRCCTRL(self._panel, 'pld_ph_xn_value')
        self._ph_yp_value = xrc.XRCCTRL(self._panel, 'pld_ph_yp_value')
        self._ph_yn_value = xrc.XRCCTRL(self._panel, 'pld_ph_yn_value')

        self._temp_supply_value = xrc.XRCCTRL(self._panel, 'pld_temp_supply_value')
        self._temp_xp_value = xrc.XRCCTRL(self._panel, 'pld_temp_xp_value')
        self._temp_xn_value = xrc.XRCCTRL(self._panel, 'pld_temp_xn_value')
        self._temp_yp_value = xrc.XRCCTRL(self._panel, 'pld_temp_yp_value')
        self._temp_yn_value = xrc.XRCCTRL(self._panel, 'pld_temp_yn_value')
        self._temp_sads_value = xrc.XRCCTRL(self._panel, 'pld_temp_sads_value')
        self._temp_sail_value = xrc.XRCCTRL(self._panel, 'pld_temp_sail_value')
        self._temp_cam_n_value = xrc.XRCCTRL(self._panel, 'pld_temp_cam_n_value')
        self._temp_cam_w_value = xrc.XRCCTRL(self._panel, 'pld_temp_cam_w_value')

    @staticmethod
    def update_textbox(control, value):
        old_text = control.GetLabel()
        new_text = str(value)
        if old_text != new_text:
            control.ChangeValue(new_text)

    def update(self):
        value = self._system.payload.timeout_callback() if self._system.payload.timeout_callback else 0
        self.update_textbox(self._timeout_value, value)

        self.update_textbox(self._whoami_value, self._system.payload.Status.WhoAmI)

        self.update_textbox(self._hk_int_value, self._system.payload.Housekeeping.INT_3V3D)
        self.update_textbox(self._hk_obc_value, self._system.payload.Housekeeping.OBC_3V3D)

        self.update_textbox(self._radfet_status_value, self._system.payload.RadFET.Status)
        self.update_textbox(self._radfet_temp_value, self._system.payload.RadFET.Temperature)
        self.update_textbox(self._radfet_vth1_value, self._system.payload.RadFET.Vth0)
        self.update_textbox(self._radfet_vth2_value, self._system.payload.RadFET.Vth1)
        self.update_textbox(self._radfet_vth3_value, self._system.payload.RadFET.Vth2)

        self.update_textbox(self._suns_v1_value, self._system.payload.SunS_Ref.V1)
        self.update_textbox(self._suns_v2_value, self._system.payload.SunS_Ref.V2)
        self.update_textbox(self._suns_v3_value, self._system.payload.SunS_Ref.V3)
        self.update_textbox(self._suns_v4_value, self._system.payload.SunS_Ref.V4)
        self.update_textbox(self._suns_v5_value, self._system.payload.SunS_Ref.V5)

        self.update_textbox(self._ph_xp_value, self._system.payload.Photodiodes.Xp)
        self.update_textbox(self._ph_xn_value, self._system.payload.Photodiodes.Xn)
        self.update_textbox(self._ph_yp_value, self._system.payload.Photodiodes.Yp)
        self.update_textbox(self._ph_yn_value, self._system.payload.Photodiodes.Yn)

        self.update_textbox(self._temp_supply_value, self._system.payload.Temperatures.Supply)
        self.update_textbox(self._temp_xp_value, self._system.payload.Temperatures.Xp)
        self.update_textbox(self._temp_xn_value, self._system.payload.Temperatures.Xn)
        self.update_textbox(self._temp_yp_value, self._system.payload.Temperatures.Yp)
        self.update_textbox(self._temp_yn_value, self._system.payload.Temperatures.Yn)
        self.update_textbox(self._temp_sads_value, self._system.payload.Temperatures.SADS)
        self.update_textbox(self._temp_sail_value, self._system.payload.Temperatures.Sail)
        self.update_textbox(self._temp_cam_n_value, self._system.payload.Temperatures.CAMNadir)
        self.update_textbox(self._temp_cam_w_value, self._system.payload.Temperatures.CAMWing)

    def root(self):
        return self._panel

    @bind('pld_timeout_value', wx.EVT_TEXT, args=(0,))
    def push_timeout(self, evt, args):
        new_value = int(evt.EventObject.GetValue())
        if new_value is None:
            self._system.payload.timeout_callback = None
        else:
            self._system.payload.timeout_callback = lambda: new_value

    @staticmethod
    def push_textbox_value(item, evt, item_field_name, max_value):
        new_text = evt.EventObject.GetValue()
        new_value = 0
        try:
            new_value = int(new_text)
        except:
            pass
        if new_value > max_value or new_value < 0:
            return
        setattr(item, item_field_name, new_value)

    @bind('pld_temp_supply_value', wx.EVT_TEXT, args=('Supply',))
    @bind('pld_temp_xp_value', wx.EVT_TEXT, args=('Xp',))
    @bind('pld_temp_xn_value', wx.EVT_TEXT, args=('Xn',))
    @bind('pld_temp_yp_value', wx.EVT_TEXT, args=('Yp',))
    @bind('pld_temp_yn_value', wx.EVT_TEXT, args=('Yn',))
    @bind('pld_temp_sads_value', wx.EVT_TEXT, args=('SADS',))
    @bind('pld_temp_sail_value', wx.EVT_TEXT, args=('Sail',))
    @bind('pld_temp_cam_n_value', wx.EVT_TEXT, args=('CAMNadir',))
    @bind('pld_temp_cam_w_value', wx.EVT_TEXT, args=('CAMWing',))
    def push_temperatures(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.Temperatures, evt, item_field_name, 65535)

    @bind('pld_ph_xp_value', wx.EVT_TEXT, args=('Xp',))
    @bind('pld_ph_xn_value', wx.EVT_TEXT, args=('Xn',))
    @bind('pld_ph_yp_value', wx.EVT_TEXT, args=('Yp',))
    @bind('pld_ph_yn_value', wx.EVT_TEXT, args=('Yn',))
    def push_photodiodes(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.Photodiodes, evt, item_field_name, 65535)

    @bind('pld_suns_v1_value', wx.EVT_TEXT, args=('V1',))
    @bind('pld_suns_v2_value', wx.EVT_TEXT, args=('V2',))
    @bind('pld_suns_v3_value', wx.EVT_TEXT, args=('V3',))
    @bind('pld_suns_v4_value', wx.EVT_TEXT, args=('V4',))
    @bind('pld_suns_v5_value', wx.EVT_TEXT, args=('V5',))
    def push_suns(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.SunS_Ref, evt, item_field_name, 65535)

    @bind('pld_radfet_status_value', wx.EVT_TEXT, args=('Status',))
    def push_radfet_status(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.RadFET, evt, item_field_name, 255)

    @bind('pld_radfet_temp_value', wx.EVT_TEXT, args=('Temperature',))
    @bind('pld_radfet_vth1_value', wx.EVT_TEXT, args=('Vth0',))
    @bind('pld_radfet_vth2_value', wx.EVT_TEXT, args=('Vth1',))
    @bind('pld_radfet_vth3_value', wx.EVT_TEXT, args=('Vth2',))
    def push_radfet(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.RadFET, evt, item_field_name, (1 << 32) - 1)

    @bind('pld_hk_int_value', wx.EVT_TEXT, args=('INT_3V3D',))
    @bind('pld_hk_obc_value', wx.EVT_TEXT, args=('OBC_3V3D',))
    def push_housekeeping(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.Housekeeping, evt, item_field_name, 65535)

    @bind('pld_whoami_value', wx.EVT_TEXT, args=('WhoAmI',))
    def push_whoami(self, evt, item_field_name):
        self.push_textbox_value(self._system.payload.Status, evt, item_field_name, 255)
