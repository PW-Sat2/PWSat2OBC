from response_frames import ResponseFrame, response_frame
from response_frames.operation import OperationSuccessFrame, OperationErrorFrame

@response_frame(0x1E)
class SetInternalDetumblingModeSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x1E)
class SetInternalDetumblingModeErrorFrame(OperationErrorFrame):
    pass

@response_frame(0x1F)
class SetAdcsModeSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x1F)
class SetAdcsModeErrorFrame(OperationErrorFrame):
    pass