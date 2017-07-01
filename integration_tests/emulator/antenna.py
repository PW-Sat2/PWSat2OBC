import wx
from wx import xrc

from .base import ModuleBase, bind

class AntennaModule(ModuleBase):
    def __init__(self, name, pos,  antenna_controller):
        self._antenna_controller = antenna_controller
        self._panel = None
        self.title = 'Antenna ({})'.format(name)
        self.grid_pos = pos

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'AntennaModule')

        self.bind_handlers()

        self._antenna_arm_status = xrc.XRCCTRL(self._panel, 'antenna_arm_status')
        self._antenna_deployment_in_progress = xrc.XRCCTRL(self._panel, 'antenna_deployment_in_progress')

        self._antenna1_status = xrc.XRCCTRL(self._panel, 'antenna1_status')
        self._antenna2_status = xrc.XRCCTRL(self._panel, 'antenna2_status')
        self._antenna3_status = xrc.XRCCTRL(self._panel, 'antenna3_status')
        self._antenna4_status = xrc.XRCCTRL(self._panel, 'antenna4_status')

    def root(self):
        return self._panel

    def update(self):
        if self._antenna_controller.armed:
            self._antenna_arm_status.SetLabel("Armed")
        else:
            self._antenna_arm_status.SetLabel("Disarmed")

        if self._antenna_controller.deployment_in_progress:
            self._antenna_deployment_in_progress.SetLabel("Deployment in progress")
        else:
            self._antenna_deployment_in_progress.SetLabel("Idle")

        self._antenna1_status.SetLabel(str(self._antenna_controller.antenna_state[0]))
        self._antenna2_status.SetLabel(str(self._antenna_controller.antenna_state[1]))
        self._antenna3_status.SetLabel(str(self._antenna_controller.antenna_state[2]))
        self._antenna4_status.SetLabel(str(self._antenna_controller.antenna_state[3]))

    @bind('antenna1_deploy', wx.EVT_BUTTON, args=(0,))
    @bind('antenna2_deploy', wx.EVT_BUTTON, args=(1,))
    @bind('antenna3_deploy', wx.EVT_BUTTON, args=(2,))
    @bind('antenna4_deploy', wx.EVT_BUTTON, args=(3,))
    def _on_antenna_deploy(self, evt, antenna_id):
        self._antenna_controller.antenna_state[antenna_id].deployed = True
        self._antenna_controller.antenna_state[antenna_id].is_being_deployed = False

    @bind('antenna1_deploy_cancel', wx.EVT_BUTTON, args=(0,))
    @bind('antenna2_deploy_cancel', wx.EVT_BUTTON, args=(1,))
    @bind('antenna3_deploy_cancel', wx.EVT_BUTTON, args=(2,))
    @bind('antenna4_deploy_cancel', wx.EVT_BUTTON, args=(3,))
    def _on_antenna_deploy_cancel(self, evt, antenna_id):
        self._antenna_controller.antenna_state[antenna_id].is_being_deployed = False
