from response_frames import response_frame
from response_frames.operation import OperationSuccessFrame, OperationErrorFrame


@response_frame(0x18)
class SetBitrateSuccessFrame(OperationSuccessFrame):
    pass

@response_frame(0x18)
class SetBitrateErrorFrame(OperationErrorFrame):
    pass
