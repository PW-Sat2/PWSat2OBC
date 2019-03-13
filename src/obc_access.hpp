#ifndef SRC_OBC_ACCESS_HPP_
#define SRC_OBC_ACCESS_HPP_

#include "antenna/fwd.hpp"
#include "boot/fwd.hpp"
#include "camera/camera.h"
#include "comm/comm.hpp"
#include "eps/fwd.hpp"
#include "fs/fwd.hpp"
#include "gyro/fwd.hpp"
#include "i2c/forward.h"
#include "imtq/fwd.hpp"
#include "obc/adcs_fwd.hpp"
#include "obc/experiments_fwd.hpp"
#include "obc/fdir_fwd.hpp"
#include "obc/hardware_fwd.hpp"
#include "obc/memory_fwd.hpp"
#include "obc/scrubbing_fwd.hpp"
#include "rtc/fwd.hpp"
#include "suns/fwd.hpp"
#include "temp/fwd.hpp"
#include "terminal/fwd.hpp"
#include "time/fwd.hpp"

AntennaDriver& GetAntennaDriver();
boot::BootSettings& GetBootSettings();
devices::comm::CommObject& GetCommDriver();
obc::PersistentStorageAccess& GetPersistentStorageAccess();
services::fs::YaffsFileSystem& GetFileSystem();
obc::FDIR& GetFDIR();
devices::eps::EPSDriver& GetEPS();
obc::OBCExperiments& GetExperiments();
obc::OBCMemory& GetMemory();
devices::imtq::ImtqDriver& GetIMTQ();
devices::suns::SunSDriver& GetSUNS();
drivers::i2c::I2CInterface& GetI2C();
devices::gyro::GyroDriver& GetGyro();
devices::rtc::RTCObject& GetRTC();
services::time::TimeProvider& GetTimeProvider();
obc::OBCScrubbing& GetScrubbing();
temp::ADCTemperatureReader& GetMCUTemperature();
devices::camera::Camera& GetCamera();
obc::Adcs& GetAdcs();

#endif /* SRC_OBC_ACCESS_HPP_ */
