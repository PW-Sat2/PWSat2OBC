from response_frames import response_frame
from response_frames.operation import OperationSuccessFrame, OperationErrorFrame


@response_frame(0x19)
class DisableOverheatSubmodeSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x19)
class DisableOverheatSubmodeErrorFrame(OperationErrorFrame):
    pass
