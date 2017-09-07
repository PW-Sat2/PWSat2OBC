from response_frames import response_frame
from response_frames.operation import OperationSuccessFrame, OperationErrorFrame


@response_frame(0x1D)
class StopSailDeploymentSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x1D)
class StopSailDeploymentErrorFrame(OperationErrorFrame):
    pass
