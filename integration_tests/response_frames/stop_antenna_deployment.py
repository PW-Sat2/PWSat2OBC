from response_frames import response_frame
from response_frames.operation import OperationSuccessFrame, OperationErrorFrame


@response_frame(0x23)
class StopAntennaDeploymentSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x23)
class StopAntennaDeploymentErrorFrame(OperationErrorFrame):
    pass
