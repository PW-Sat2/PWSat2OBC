from response_frames import ResponseFrame, response_frame
from enum import unique, IntEnum

@unique
class DownlinkApid(IntEnum):
    Pong = 0x01,
    Operation = 0x2,
    ErrorCounters = 0x3,
    ProgramUpload = 0x4,
    PeriodicMessage = 0x5,
    PersistentState = 0x6,
    BootSlotsInfo = 0x7,
    CompileInfo = 0x8,
    EraseFlash = 0x9,
    FileRemove = 0xA,
    FileSend = 0xB,
    FileList = 0xC,
    Forbidden = 0xD,
    Photo = 0xE,
    SunS = 0xF,
    Experiment = 0x10,
    ErrorCounterConfiguration = 0x11,
    PurgePhoto = 0x12,
    Powercycle = 0x13,
    Sail = 0x14,
    TimeCorrection = 0x15,
    TimeSet = 0x16,
    Comm = 0x17,
    SetBitrate = 0x18,
    DisableOverheatSubmode = 0x19,
    I2C = 0x1A,
    PeriodicSet = 0x1B,
    SailExperiment = 0x1C,

@response_frame(0)
class GenericSuccessResponseFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] == 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.response = self.payload()[2:]


@response_frame(0)
class GenericErrorResponseFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 2 and payload[1] != 0

    def decode(self):
        self.correlation_id = self.payload()[0]
        self.error_code = self.payload()[1]
        self.response = self.payload()[2:]


@response_frame(DownlinkApid.Experiment)
class ExperimentSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.Experiment)
class ExperimentErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.Comm)
class CommSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.Photo)
class PhotoSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.Photo)
class PhotoErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.I2C)
class I2CSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.FileSend)
class FileSendSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.FileSend)
class FileSendErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.FileList)
class FileListErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.FileRemove)
class FileRemoveSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.FileRemove)
class FileRemoveErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.PurgePhoto)
class PurgePhotoSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.Powercycle)
class PowerSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.Sail)
class SailSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.TimeCorrection)
class TimeCorrectionSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.TimeSet)
class TimeSetSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.TimeSet)
class TimeSetErrorFrame(GenericErrorResponseFrame):
    pass


@response_frame(DownlinkApid.ErrorCounterConfiguration)
class ErrorCounterConfigurationSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.BootSlotsInfo)
class BootSlotsInfoSuccessFrame(GenericSuccessResponseFrame):
    pass